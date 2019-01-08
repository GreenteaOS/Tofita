SECTIONS
{
	/* Start putting sections at 1MiB, conventional place for bootloaders */
	. = 1M;

	.text :
	{
		*(.head)
		*(.text)
	}

	/* Read-write data (initialized) */
	.data : ALIGN(0x1000)
	{
		*(.data)
	}

	.bss :
	{
		*(.stack)
		*(.bss)
		_lower_kernel_end = .;
	}
}
