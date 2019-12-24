// The Tofita Kernel
// Copyright (C) 2019  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Based on https://gitlab.com/nagisa/huehuehuehuehue/blob/master/src/serial.c

#define SERIAL_DUMP_HEX(expr) \
serialPrint(#expr " = "); \
serialPrintHex((uint64_t) (expr)); \
serialPrint("\n");

#define SERIAL_DUMP_HEX0(expr) \
serialPrint(#expr " = "); \
serialPrintHex((uint64_t) (expr));

#define SERIAL_DUMP_BITS(expr) \
serialPrint(#expr " -> "); \
serialPrintBits((uint64_t) (expr)); \
serialPrint("\n"); \

#define SERIAL_REGISTER_BASE 0x03F8
#define SERIAL_BAUD_RATE 115200
#define SERIAL_CLOCK_RATE 1843200
#define SERIAL_LINE_CONTROL 0x03
#define SERIAL_REGISTER_STRIDE 1
#define SERIAL_FIFO_CONTROL 0x07
#define SERIAL_EXTENDED_FIFO_TX_SIZE 64
#define SERIAL_USE_HW_FLOW_CONTROL false
#define SERIAL_DETECT_CABLE false

// 16550 UART register offsets and bitfields
#define R_UART_RXBUF          0
#define R_UART_TXBUF          0
#define R_UART_BAUD_LOW       0
#define R_UART_BAUD_HIGH      1
#define R_UART_FCR            2
#define   B_UART_FCR_FIFOE    (1 << 0)
#define   B_UART_FCR_FIFO64   (1 << 5)
#define R_UART_LCR            3
#define   B_UART_LCR_DLAB     (1 << 7)
#define R_UART_MCR            4
#define   B_UART_MCR_DTRC     (1 << 0)
#define   B_UART_MCR_RTS      (1 << 1)
#define R_UART_LSR            5
#define   B_UART_LSR_RXRDY    (1 << 0)
#define   B_UART_LSR_TXRDY    (1 << 5)
#define   B_UART_LSR_TEMT     (1 << 6)
#define R_UART_MSR            6
#define   B_UART_MSR_CTS      (1 << 4)
#define   B_UART_MSR_DSR      (1 << 6)
#define   B_UART_MSR_RI       (1 << 7)
#define   B_UART_MSR_DCD      (1 << 8)

void *tmemset(void *dest, int e, size_t len) {
	uint8_t *d = (uint8_t *)dest;
	for(uint64_t i = 0; i < len; i++, d++) {
		*d = e;
	}
}

// Solve conflict with gnu-efi
#ifdef TOFITA
void *memset(void *dest, int e, size_t len) {
	uint8_t *d = (uint8_t *)dest;
	for(uint64_t i = 0; i < len; i++, d++) {
		*d = e;
	}
	return dest;
}
#endif

uint64_t kstrlen(const uint8_t *data) {
	uint64_t r;
	for(r = 0; *data != 0; data++, r++);
	return r;
}

static inline uint8_t portInb(uint16_t port) {
	uint8_t data;
	__asm__ volatile("inb %w1,%b0" : "=a" (data) : "d"(port));
	return data;
}

static inline uint8_t portOutb(uint16_t port, uint8_t value) {
	__asm__ volatile("outb %b0,%w1" : : "a" (value), "d"(port));
	return value;
}

uint8_t readSerialRegister(uint16_t offset) {
	return portInb(SERIAL_REGISTER_BASE + offset * SERIAL_REGISTER_STRIDE);
}

function writeSerialRegister(uint16_t offset, uint8_t d) {
	portOutb(SERIAL_REGISTER_BASE + offset * SERIAL_REGISTER_STRIDE, d);
}

bool serialPortWritable() {
	if (SERIAL_USE_HW_FLOW_CONTROL) {
		if (SERIAL_DETECT_CABLE) {
			// Wait for both DSR and CTS to be set
			//   DSR is set if a cable is connected.
			//   CTS is set if it is ok to transmit data
			//
			//   DSR  CTS  Description                               Action
			//   ===  ===  ========================================  ========
			//    0    0   No cable connected.                       Wait
			//    0    1   No cable connected.                       Wait
			//    1    0   Cable connected, but not clear to send.   Wait
			//    1    1   Cable connected, and clear to send.       Transmit
			return (bool)((readSerialRegister(R_UART_MSR) & (B_UART_MSR_DSR | B_UART_MSR_CTS))
						   == (B_UART_MSR_DSR | B_UART_MSR_CTS));
		} else {
			// Wait for both DSR and CTS to be set OR for DSR to be clear.
			//   DSR is set if a cable is connected.
			//   CTS is set if it is ok to transmit data
			//
			//   DSR  CTS  Description                               Action
			//   ===  ===  ========================================  ========
			//    0    0   No cable connected.                       Transmit
			//    0    1   No cable connected.                       Transmit
			//    1    0   Cable connected, but not clear to send.   Wait
			//    1    1   Cable connected, and clar to send.        Transmit
			return (bool)((readSerialRegister(R_UART_MSR) & (B_UART_MSR_DSR | B_UART_MSR_CTS))
						   != (B_UART_MSR_DSR));
		}
	}
	return true;
}

uint64_t serialPortWrite(uint8_t *buffer, uint64_t size) {
	if (buffer == NULL) { return 0; }
	if (size == 0) {
		// Flush the hardware
		//
		// Wait for both the transmit FIFO and shift register empty.
		while ((readSerialRegister(R_UART_LSR) & (B_UART_LSR_TEMT | B_UART_LSR_TXRDY))
				!= (B_UART_LSR_TEMT | B_UART_LSR_TXRDY));
		while (!serialPortWritable());
		return 0;
	}
	// Compute the maximum size of the Tx FIFO
	uint64_t fifoSize = 1;
	if ((SERIAL_FIFO_CONTROL & B_UART_FCR_FIFOE) != 0) {
		if ((SERIAL_FIFO_CONTROL & B_UART_FCR_FIFO64) == 0) {
			fifoSize = 16;
		} else {
			fifoSize = SERIAL_EXTENDED_FIFO_TX_SIZE;
		}
	}
	while (size != 0) {
		// Wait for the serial port to be ready, to make sure both the transmit FIFO
		// and shift register empty.
		while ((readSerialRegister(R_UART_LSR) & B_UART_LSR_TEMT) == 0);
		// Fill then entire Tx FIFO
		for (uint64_t index = 0; index < fifoSize && size != 0; index++, size--, buffer++) {
			// Wait for the hardware flow control signal
			while (!serialPortWritable());
			// Write byte to the transmit buffer.
			writeSerialRegister(R_UART_TXBUF, *buffer);
		}
	}
	return size;
}

function serialPrint(const char8_t *print) {
	serialPortWrite((uint8_t *)print, kstrlen((uint8_t *)print));
}

function serialPrintln(const char8_t *print) {
	serialPrint(print);
	serialPrint(u8"\n");
}

function serialPrintInt(uint64_t n) {
	uint8_t buf[24];
	for (uint8_t i = 0; i < 24; i++) buf[i] = 0;
	uint8_t *bp = buf + 24;
	do {
		bp--;
		*bp = '0' + n % 10;
		n /= 10;
	} while (n != 0);
	serialPortWrite((uint8_t *)bp, buf - bp + 24);
}

function serialPrintHex(uint64_t n) {
	serialPrint(u8"0x");
	uint8_t buf[16], *bp = buf + 16;
	for(int i = 0; i < 16; i++) buf[i] = '0';
	do {
		bp--;
		uint8_t mod = n % 16;
		if(mod < 10) {
			*bp = '0' + mod;
		} else {
			*bp = 'A' - 10 + mod;
		}
		n /= 16;
	} while (n != 0);
	serialPortWrite((uint8_t *)buf, 16);
}

function serialPrintMem(const void *mem, int n)
{
	serialPrint(u8"@");
	serialPrintHex((uint64_t) mem);
	serialPrint(u8"\n");
	const uint8_t *memc = (const uint8_t *) mem;
	for (int32_t i = 0; i < n; ++i) {
		if (i) serialPrint(u8":");
		serialPrintHex(memc[i]);
	}
}

function serialPrintPtr(void *ptr)
{
	serialPrintHex((uint64_t) ptr);
}

function serialPrintBits(uint64_t value)
{
	for(int i = 0; i < 64; ++i) {
		if(value & (1ull << i)) {
			serialPrintInt(i);
			serialPrint(u8";");
		}
	}
}

int32_t __cdecl putchar(uint8_t c) {
	char8_t buffer[] = {c, 0};
	serialPrint(buffer);
	return c;
}

#define EOF 0
int puts(const uint8_t *string)
{
	int i = 0;
	while (string[i]) //standard c idiom for looping through a null-terminated string
	{
		if (putchar(string[i]) == EOF) //if we got the EOF value from writing the uint8_t
		{
			return EOF;
		}
		i++;
	}
	return 1; //to meet spec.
}

uint8_t* comItoA(int i, uint8_t b[]){
	uint8_t const digit[] = "0123456789";
	uint8_t* p = b;
	if (i<0) {
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do { //Move to where representation ends
		++p;
		shifter = shifter/10;
	} while(shifter);
	*p = '\0';
	do { //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	} while(i);
	return b;
}

function serialPrintf(const char8_t *c, ...) {
	uint8_t *s;
	va_list lst;
	va_start(lst, c);
	while (*c != '\0') {
		if (*c != '%') {
			putchar(*c);
			c++;
			continue;
		}

		c++;

		if (*c == '\0') {
			break;
		}

		switch (*c)
		{
			case 's': puts(va_arg(lst, uint8_t *)); break;
			case 'c': putchar(va_arg(lst, int)); break;
			case 'd': {
				int value = va_arg(lst, int);
				uint8_t buffer[16];
				for (uint8_t i = 0; i < 16; i++) buffer[i] = 0;
				comItoA(value, buffer);
				uint8_t *c = buffer;
				while (*c != '\0') {
					putchar(*c);
					c++;
				}
				break;
			}
		}
		c++;
	}
}
