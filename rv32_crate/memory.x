/* memory.x -- 1 KiB of RAM, code and data all in the same place */

MEMORY
{
  /* RAM at 0x00000000, 1 KiB long */
  RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 1K
}

/* Map all logical regions to the single RAM region */
REGION_ALIAS("REGION_TEXT",   RAM);
REGION_ALIAS("REGION_RODATA", RAM);
REGION_ALIAS("REGION_DATA",   RAM);
REGION_ALIAS("REGION_BSS",    RAM);
REGION_ALIAS("REGION_HEAP",   RAM);
REGION_ALIAS("REGION_STACK",  RAM);

PROVIDE(_max_hart_id     = 0);
PROVIDE(_hart_stack_size = 256);
