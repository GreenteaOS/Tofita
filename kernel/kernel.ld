SECTIONS
{
	. = 0xffff800000000000;

	.text :
	{
		*(.head1)
		*(.text)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(.stack1)
		*(.bss)
	}
}
