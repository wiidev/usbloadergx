OUTPUT_FORMAT("elf32-bigarm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
 
 
/*
	Change exe start and ram start as needed for the custom IOS module.
	Current settings are for the freespace after OH1 module from IOS 31.
 */

MEMORY {
	ios_info_table	: ORIGIN = 0x0, LENGTH = 0xf4
	exe(rwx)	: ORIGIN = 0x138c0000, LENGTH = 0x4000
	
	ram(rw)		: ORIGIN = 0x138c8000, LENGTH = 0x8000
	
	ios_exe(rw)	: ORIGIN = 0x13700000, LENGTH = 0x80000
	
}



__exe_start_virt__			= 0x138c0000;
__exe_start_phys__			= 0x138c0000;
__ram_start_virt__			= 0x138c8000;
__ram_start_phys__			= 0x138c8000;
__ios_info_table_start		= 0x0;



SECTIONS
{
    .debug_aranges 0 : { *(.debug_aranges) } .debug_pubnames 0 : { *(.debug_pubnames) } .debug_info 0 : { *(.debug_info) } .debug_abbrev 0 : { *(.debug_abbrev) } .debug_line 0 : { *(.debug_line) } .debug_frame 0 : { *(.debug_frame) } .debug_str 0 : { *(.debug_str) } .debug_loc 0 : { *(.debug_loc) } .debug_macinfo 0 : { *(.debug_macinfo) } .note.arm.ident 0 : { KEEP (*(.note.arm.ident)) }
    

    .mem_exe 0x13700000 : {
		 *(.mem_exe) 
	} > ios_exe


    .ios_info_table __ios_info_table_start :  { . = .; KEEP (*(.ios_info_table)) } > ios_info_table
    .init __exe_start_virt__ : AT (__exe_start_phys__) { . = .; KEEP (*(.init)) } > exe
    .text ALIGN (0x4) : { 
    	*(.text*) 
    	*(.gnu.warning) 
    	*(.gnu.linkonce.t.*) 
    	*(.init) 
    	*(.glue_7) 
    	*(.glue_7t) } > exe
    .data __ram_start_virt__ : AT (__ram_start_phys__) { KEEP( *(.ios_data) ) *(.data*) *(.data1) *(.gnu.linkonce.d.*)  . = ALIGN (4);  __CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) __CTOR_END__ = ABSOLUTE (.); __DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) __DTOR_END__ = ABSOLUTE (.); *(.dynamic) *(.sdata*) *(.gnu.linkonce.s.*) . = ALIGN (4); *(.2ram.*) } > ram 
    .fini : { . = .; *(.fini) } > ram
     .rodata ALIGN (0x4) : { 
    	. = .; 
    	*(.rodata*) 
    	*(.gnu.linkonce.r.*) } > ram
    .rodata1 ALIGN (0x4) : { . = .; *(.rodata1) } > ram
    .fixup ALIGN (0x4) : { . = .; *(.fixup) } > ram
    .gcc_except_table ALIGN (0x4) : { . = .; *(.gcc_except_table) } > ram
    .got ALIGN (0x4) : { *(.got.plt) *(.got) } > ram
    .bss ALIGN (0x4) : { *(.scommon) *(.dynsbss) *(.sbss*) *(.gnu.linkonce.sb.*) *(.dynbss) *(.bss*) *(.gnu.linkonce.b.*) *(COMMON) KEEP( *(.ios_bss) ) } > ram
  	. = ALIGN(4);
     
}
