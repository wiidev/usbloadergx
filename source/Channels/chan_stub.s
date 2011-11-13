#define STUB 0x3400

        .text
        .section .text
        .globl _unstub_start

_unstub_start:
        isync
        // set MSR[DR:IR] = 00, jump to STUB
        lis 3,STUB@h
        ori 3,3,STUB@l
        mtsrr0 3

        mfmsr 3
        li 4,0x30
        andc 3,3,4
        mtsrr1 3
        rfi
