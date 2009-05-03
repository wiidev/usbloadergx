
	# (c) Nuke www.usbgecko.com. Licensed under GPL V2
.text
#include "ppc.h"


.globl patchhook		# r3 address
patchhook:
	mtctr	r4
	lis	r6, 0x4E80
	ori	r6, r6, 0x0020	# blr
findblr:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writebranch
	addi	r3, r3, 4	# next word
	bdnz	findblr		# loop length
	b	exit		# stop unhooked game hanging

writebranch:
	lis     r4, 0x8000	# 800018A0 hook location (source)
	ori     r4, r4, 0x18A8
	subf    r4, r3, r4	# subtract r3 from r4 and place in r4
	lis     r5, 0x3FF	
	ori     r5, r5, 0xFFFF	# 0x3FFFFFF
	and     r4, r4, r5
	lis     r5, 0x4800	# 0x48000000
	or      r4, r4, r5
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exit:
	blr                     # return

.globl patchhook2		# r3 address
patchhook2:
	mtctr	r4
	lis	r6, 0x4E80
	ori	r6, r6, 0x0020	# blr
findblr2:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writebranch2
	addi	r3, r3, 4	# next word
	bdnz	findblr2		# loop length
	b	exit2		# stop unhooked game hanging

writebranch2:
	lis     r4, 0x8000	# 81700000 our temp patcher
	ori     r4, r4, 0x18a8
	subf    r4, r3, r4	# subtract r3 from r4 and place in r4
	lis     r5, 0x3FF	
	ori     r5, r5, 0xFFFF	# 0x3FFFFFF
	and     r4, r4, r5
	lis     r5, 0x4800	# 0x48000000
	or      r4, r4, r5
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exit2:
	blr                     # return

.globl patchhook3		# r3 address
patchhook3:
	mtctr	r4
	lis	r6, 0x4BFF
	ori	r6, r6, 0xE955	# blr
findbne:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writebl
	addi	r3, r3, 4	# next word
	bdnz	findbne		# loop length
	b	exit3		# stop unhooked game hanging

writebl:
	lis     r4, 0x4BFF	# 81700000 our temp patcher
	ori     r4, r4, 0xEA91
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exit3:
	blr                     # return

.globl patchhook4		# r3 address
patchhook4:
	mtctr	r4
	lis	r6, 0x4082
	ori	r6, r6, 0x001C	# blr
findregion:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writebr
	addi	r3, r3, 4	# next word
	bdnz	findregion	# loop length
	b	exit4		# stop unhooked game hanging

writebr:
	lis     r4, 0x4800	
	ori     r4, r4, 0x001C
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exit4:
	blr                     # return

.globl multidolpatchone		# r3 address
multidolpatchone:
	mtctr	r4
	lis	r6, 0x3800
	ori	r6, r6, 0x0001	# (li r0,1) 
findmulti:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writemulti
	subi	r3, r3, 4	# go back
	bdnz	findmulti	# loop length
	b	exit5		# stop unhooked game hanging

writemulti:
	lis     r4, 0x8170	# 81700000
	ori     r4, r4, 0x0020
	subf	r18, r3, r4	# subf r18,(source),(dest)
	lis	r6, 0x4800
	ori	r6,r6,1
	rlwimi	r6,r18,0,6,29
	stw	r6,0(r3)
	stw	r6,0(r19)
	stw	r3,4(r19)
	dcbf r0, r3
	sync
	icbi r0, r3
	isync
exit5:
	blr                     # return

.globl multidolpatchtwo		# r3 address
multidolpatchtwo:
	mtctr	r4
	lis	r6, 0x3F60
	ori	r6, r6, 0x8000	# (lis r27,-32768)  
findmulti2:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writemulti2
	addi	r3, r3, 4	# go forward
	bdnz	findmulti2	# loop length
	b	exit6		# stop unhooked game hanging

writemulti2:
	lis     r4, 0x8170	# 81700020
	ori     r4, r4, 0x0000
	subf	r18, r3, r4	# subf r18,(source),(dest)
	lis	r6, 0x4800
	ori	r6,r6,1
	rlwimi	r6,r18,0,6,29
	stw	r6,0(r3)
	stw	r6,0(r19)
	stw	r3,4(r19)
	dcbf r0, r3
	sync
	icbi r0, r3
	isync
exit6:
	blr                     # return

.globl langvipatch		# r3 address, r4 len, r5 lang byte
langvipatch:
	mtctr	r4
	lis	r6, 0x8861 
	ori	r6, r6, 0x0008	# lbz     r3, 8(sp)
findlang:
	lwz	r7, 0(r3)
	cmpw	r6, r7
	beq	patchlang
	addi	r3, r3, 4	# next word
	bdnz	findlang	# loop length
	b	exitlang	# stop unhooked game hanging

patchlang:
	
	lis     r4, 0x3860	# 0x38600001	 li      %r3, 1          # eng
	add     r4, r4, r5
gofinal:
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitlang:
	blr                     # return

.globl vipatch		# r3 address
vipatch:
	mtctr	r4
	lis	r6, 0x5400  
	ori	r6, r6, 0xFFFE
findvi:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	patchvi
	addi	r3, r3, 4	# next word
	bdnz	findvi		# loop length
	b	exitvi		# stop unhooked game hanging

patchvi:
	lis	r4, 0x8000
	ori	r4, r4, 0x0003
	lbz	r5, 0(r4)
	cmpwi	r5, 0x45	# USA
	beq	patchusa
	cmpwi	r5, 0x4A
	beq	patchjap2	# JAP
	b	exitvi
patchjap2:
	lis     r4, 0x3800	
	ori     r4, r4, 0x0001
	b	gofinal2
patchusa:
	lis     r4, 0x3800	
	ori     r4, r4, 0x0000
gofinal2:
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitvi:
	blr                     # return

.globl regionfreejap		# r3 address
regionfreejap:
	mtctr	r4
	lis	r6, 0x2C1B
	ori	r6, r6, 0x0000	# blr
findjap:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writenop
	addi	r3, r3, 4	# next word
	bdnz	findjap		# loop length
	b	exitjap		# stop unhooked game hanging

writenop:
	addi	r3, r3, 4	# next word
	lis     r4, 0x6000	# nop
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitjap:
	blr                     # return

.globl regionfreeusa		# r3 address
regionfreeusa:
	mtctr	r4
	lis	r6, 0x281B
	ori	r6, r6, 0x0001	# blr
findusa:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writenop1
	addi	r3, r3, 4	# next word
	bdnz	findusa		# loop length
	b	exitusa		# stop unhooked game hanging

writenop1:
	addi	r3, r3, 4	# next word
	lis     r4, 0x6000	# nop
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitusa:
	blr                     # return

.globl regionfreepal		# r3 address
regionfreepal:
	mtctr	r4
	lis	r6, 0x281B
	ori	r6, r6, 0x0002	# blr
findpal:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writenop2
	addi	r3, r3, 4	# next word
	bdnz	findpal		# loop length
	b	exitpal		# stop unhooked game hanging

writenop2:
	addi	r3, r3, 4	# next word
	lis     r4, 0x6000	# nop
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3

	lis	r6, 0x4082
	ori	r6, r6, 0x001C	# bne     loc_81377A2C
findextra:			#this is just the bne to b patch
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writeb
	addi	r3, r3, 4	# next word
	bdnz	findextra		# loop length
	b	exitpal		# stop unhooked game hanging

writeb:
	addi	r3, r3, 4	# next word
	lis     r4, 0x4800
	ori     r4, r4, 0x001c  # b     loc_81377A2C
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitpal:
	blr                     # return

.globl removehealthcheck		# r3 address
removehealthcheck:
	mtctr	r4
	lis	r6, 0x4182
	ori	r6, r6, 0x004C 	# blr
findhe:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writebhe
	addi	r3, r3, 4	# next word
	bdnz	findhe	# loop length
	b	exithe		# stop unhooked game hanging

writebhe:
	lis     r4, 0x6000
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exithe:
	blr                     # return

	

.globl patchupdatecheck		# r3 address
patchupdatecheck:
	mtctr	r4
	lis	r6, 0x4082
	ori	r6, r6, 0x0020 	# blr
finduc:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writenopuc
	addi	r3, r3, 4	# next word
	bdnz	finduc	# loop length
	b	exituc		# stop unhooked game hanging

writenopuc:
	lis     r4, 0x6000
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exituc:
	blr                     # return




.globl copyflagcheck1	# r3 address
copyflagcheck1:
	mtctr	r4
	lis	r6, 0x5400
	ori	r6, r6, 0x07FF
findncf1:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writencf1
	subi	r3, r3, 4	# next word
	bdnz	findncf1	# loop length
	b	exitncf1		# stop unhooked game hanging

writencf1:
	lis     r4, 0x7C00
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitncf1:
	blr                     # return

.globl copyflagcheck2	# r3 address
copyflagcheck2:
	mtctr	r4
	lis	r6, 0x5400
	ori	r6, r6, 0x07FF
findncf2:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writencf2
	subi	r3, r3, 4	# next word
	bdnz	findncf2	# loop length
	b	exitncf2		# stop unhooked game hanging

writencf2:
	lis     r4, 0x7C00
	ori     r4, r4, 0x0000
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitncf2:
	blr                     # return


.globl copyflagcheck3	# r3 address
copyflagcheck3:
findncf3:
	addi	r3, r3, 20	# go back one dword (4 bytes)
	lwz	r5, 0(r3)
writencf3:
	lis     r4, 0x3860
	ori     r4, r4, 0x0001  # li r3,1
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitncf3:
	blr                     # return


.globl copyflagcheck4	# r3 address
copyflagcheck4:
	mtctr	r4
	lis	r6, 0x3BE0 
	ori	r6, r6, 0x0001 # li r31,1
findncf4:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writencf4
	addi	r3, r3, 4	# next word
	bdnz	findncf4	# loop length
	b	exitncf4	# stop unhooked game hanging

writencf4:
	lis     r4, 0x3BE0
	ori     r4, r4, 0x0000	# change this to 3BE00000 (li r31,0)
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitncf4:
	blr                     # return

.globl copyflagcheck5	# r3 address
copyflagcheck5:
	mtctr	r4
	lis	r6, 0x4182
	ori	r6, r6, 0x0024	# beq     loc_8134AA60
findncf5:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writencf5
	addi	r3, r3, 4	# next word
	bdnz	findncf5	# loop length
	b	exitncf5	# stop unhooked game hanging

writencf5:
	#addi	r3, r3, 8	# skip 2
	
	lis     r4, 0x801D
	ori     r4, r4, 0x0024	# change to 801D0024 (lwz r0,36(r29))
	stw     r4, 0(r3)	
	dcbf    r0, r3		
	icbi    r0, r3
	
	addi	r3, r3, 4	# next word

	lis     r4, 0x5400
	ori     r4, r4, 0x003C	# change to 5400003C (rlwinm r0,r0,0,0,30)
	stw     r4, 0(r3)	
	dcbf    r0, r3		
	icbi    r0, r3

	addi	r3, r3, 4	# next word

	lis     r4, 0x901D
	ori     r4, r4, 0x0024	# change to 901D0024 (stw r0,36(r29))
	stw     r4, 0(r3)	
	dcbf    r0, r3		
	icbi    r0, r3

	addi	r3, r3, 4	# next word

	lis     r4, 0x4800
	ori     r4, r4, 0x0018	# change to 48000018 (b 0x8134aa60)
	stw     r4, 0(r3)	
	dcbf    r0, r3		
	icbi    r0, r3
exitncf5:
	blr                     # return

.globl movedvdhooks	# r3 address
movedvdhooks:
	lis	r6, 0x4182 
	ori	r6, r6, 0x0120 # beq     loc_813A7938
findmd1:
	addi	r3, r3, 4	# next word
	lwz	r5, 0(r3)
writemd1:
	lis     r4, 0x6000
	ori     r4, r4, 0x0000	# nop
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exitmd1:
	blr                     # return

