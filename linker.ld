MEMORY
{
    FLASH           (RX)  : ORIGIN = 0x00000000, LENGTH = 0x00003C00
    CFG             (R)   : ORIGIN = 0x00003C00, LENGTH = 0x00003FF8
    SRAM            (RWX) : ORIGIN = 0x20000000, LENGTH = 0x00000800
    BCR_CONFIG      (R)   : ORIGIN = 0x41C00000, LENGTH = 0x00000080
    BSL_CONFIG      (R)   : ORIGIN = 0x41C00100, LENGTH = 0x00000080
}

ENTRY(reset_handler)

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))
		. = ALIGN(4);
		*(.text*)
	} >FLASH

	.rodata :
	{
		*(.rodata*)
		. = ALIGN(4);
		_etext = .;
	} >FLASH

	.data :
	{
		_sdata = .;
		KEEP(*(.data*))
		. = ALIGN(4);
		_edata = .;
	} >SRAM AT> FLASH

	.config (NOLOAD) :
	{
		_sconfig = .;
		*(.config*)
		. = ALIGN(8);
		_econfig = .;
	} >CFG

	.bss :
	{
		_sbss = .;
		*(.bss*)
		. = ALIGN(4);
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >SRAM

	.noinit :
	{
		*(.noinit*)
		. = ALIGN(4);
	} >SRAM

	_sstack = .;
	PROVIDE(_estack = ORIGIN(SRAM) + LENGTH(SRAM));

	.bcrconfig :
	{
		KEEP(*(.bcrconfig))
	} >BCR_CONFIG

	.bslconfig :
	{
		KEEP(*(.bslconfig))
	} >BSL_CONFIG
}

_end = .;
