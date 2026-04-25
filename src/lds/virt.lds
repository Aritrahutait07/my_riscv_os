OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x80000000;

  .text : {
     _text_start = .; 
    *(.text.init)
    *(.text .text.*)
    _text_end = .; 
  }

  /* ADD THIS LINE RIGHT HERE */
  PROVIDE(_global_pointer = . + 0x800);

  .rodata : {
    _rodata_start = .;
    *(.rodata .rodata.*)
    _rodata_end = .;
  }

  .data : {
    _data_start = .; 
    *(.data .data.*)
    _data_end = .; 
  }

  .bss : {
    _bss_start = .;
    *(.bss .bss.*)
    _bss_end = 
    .;
  }
  

  . = ALIGN(4096);
  _stack_start = .;    /* <--- ADD THIS SO THE MMU CAN FIND IT */
  
  /* Give the OS a generous 128KB Stack */
  . += 0x20000;        
  _stack_end = .;

  /* 2. ALIGN TO 4096 BEFORE THE HEAP */
  . = ALIGN(4096);     /* <--- CRITICAL FOR PAGE ALLOCATOR */
  _heap_start = .;
  
  _memory_end = 0x88000000;
  _heap_size  = _memory_end - _heap_start;
}