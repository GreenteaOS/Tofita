#include "log.c"

uint8_t initSerial()
{
	// Calculate divisor for baud generator
	//    Ref_Clk_Rate / Baud_Rate / 16
	uint32_t divisor = SERIAL_CLOCK_RATE / (SERIAL_BAUD_RATE * 16);
	if ((SERIAL_CLOCK_RATE % (SERIAL_BAUD_RATE * 16)) >= SERIAL_BAUD_RATE * 8) {
		divisor++;
	}
	// See if the serial port is already initialized
	bool initialized = true;
	if ((readSerialRegister(R_UART_LCR) & 0x3F)
		!= (SERIAL_LINE_CONTROL & 0x3F)) {
		initialized = false;
	}
	// Calculate current divisor
	writeSerialRegister(R_UART_LCR,
		readSerialRegister(R_UART_LCR) | B_UART_LCR_DLAB);
	uint32_t currentDivisor = readSerialRegister(R_UART_BAUD_HIGH) << 8;
	currentDivisor |= (uint32_t)readSerialRegister(R_UART_BAUD_LOW);
	writeSerialRegister(R_UART_LCR,
		readSerialRegister(R_UART_LCR) & ~B_UART_LCR_DLAB);
	// Fast path
	initialized = initialized && currentDivisor == divisor;
	if (initialized) {
		return 1;
	}
	// Wait for the serial port to be ready.
	// Verify that both the transmit FIFO and the shift register are empty.
	while ((readSerialRegister(R_UART_LSR)
				& (B_UART_LSR_TEMT | B_UART_LSR_TXRDY))
			!= (B_UART_LSR_TEMT | B_UART_LSR_TXRDY));
	// Configure baud rate
	writeSerialRegister(R_UART_LCR, B_UART_LCR_DLAB);
	writeSerialRegister(R_UART_BAUD_HIGH, (uint8_t) (divisor >> 8));
	writeSerialRegister(R_UART_BAUD_LOW, (uint8_t) (divisor & 0xff));
	// Clear DLAB and configure Data Bits, Parity, and Stop Bits.
	// Strip reserved bits from PcdSerialLineControl
	writeSerialRegister(R_UART_LCR, (uint8_t)(SERIAL_LINE_CONTROL & 0x3F));
	// Enable and reset FIFOs
	// Strip reserved bits from PcdSerialFifoControl
	writeSerialRegister(R_UART_FCR, 0x00);
	writeSerialRegister(R_UART_FCR,
						  (uint8_t)(SERIAL_LINE_CONTROL & (B_UART_FCR_FIFOE | B_UART_FCR_FIFO64)));
	// Put Modem Control Register(MCR) into its reset state of 0x00.
	writeSerialRegister(R_UART_MCR, 0x00);
	return 1;
}
