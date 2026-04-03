OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x80000000;

  .text : {
    *(.text.init)
    *(.text .text.*)
  }

  /* ADD THIS LINE RIGHT HERE */
  PROVIDE(_global_pointer = . + 0x800);

  .rodata : {
    *(.rodata .rodata.*)
  }

  .data : {
    *(.data .data.*)
  }

  .bss : {
    _bss_start = .;
    *(.bss .bss.*)
    _bss_end = 
    .;
  }
  

  . = ALIGN(16);
  . += 0x10000; 
  _stack_end = .;
  _heap_start = _stack_end;
  _memory_end = 0x88000000;
  _heap_size  = _memory_end - _heap_start;
}