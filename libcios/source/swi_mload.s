/* SWI service from dev/mload (c) 2009 Hermes / www.elotrolado.net */


	.align 4
	.arm
	.code 32
	.global swi_mload_func
swi_mload_func:
   
	svc 0xcc
	bx   lr


