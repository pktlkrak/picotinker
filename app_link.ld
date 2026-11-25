SECTIONS
{
  . = 0x0;
  .text : { *(.text) }
  .dynamic : { *(.dynamic) }
  .got : { *(.got) }
  .got.plt : { *(.got.plt) }
  .rodata : { *(.rodata) }
  .data.rel.ro : { *(.data.rel.ro) }
  . = 0x20060000;
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = 0xF0000000;
  .dynstr : { *(.dynstr) } # Discard
}
