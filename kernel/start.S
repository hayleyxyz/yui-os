#include <multiboot.h>
#include <asm.h>
#include <gdt.h>
#include <cpu.h>

#define MULTIBOOT_HEADER_FLAGS	(MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE)
#define MULTIBOOT_CHECKSUM     	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

#define KERNEL_LOAD_ADDRESS		0x00100000

#define X86_CR4_PAE                     0x00000020 /* PAE paging */
#define X86_CR4_PGE                     0x00000080 /* page global enable */

#define X86_EFER_LME                    0x00000100 /* long mode enable */
#define X86_MSR_IA32_EFER               0xc0000080 /* EFER */

#define X86_MMU_PG_P            0x0001          /* P    Valid                   */
#define X86_MMU_PG_RW           0x0002          /* R/W  Read/Write              */
#define X86_MMU_PG_U            0x0004          /* U/S  User/Supervisor         */
#define X86_MMU_PG_WT           0x0008          /* WT   Write-through           */
#define X86_MMU_PG_CD           0x0010          /* CD   Cache disable           */
#define X86_MMU_PG_A            0x0020          /* A    Accessed                */
#define X86_MMU_PG_D            0x0040          /* D    Dirty                   */
#define X86_MMU_PG_PS           0x0080          /* PS   Page size (0=4k,1=4M)   */
#define X86_MMU_PG_PTE_PAT      0x0080          /* PAT  PAT index for 4k pages  */
#define X86_MMU_PG_LARGE_PAT    0x1000          /* PAT  PAT index otherwise     */
#define X86_MMU_PG_G            0x0100          /* G    Global                  */
#define X86_DIRTY_ACCESS_MASK   0xf9f

#define PHYS_ADDR_DELTA 0

/* default flags for inner page directory entries */
#define X86_KERNEL_PD_FLAGS (X86_MMU_PG_RW | X86_MMU_PG_P | X86_MMU_PG_U)

/* default flags for 2MB/4MB/1GB page directory entries */
#define X86_KERNEL_PD_LP_FLAGS (X86_MMU_PG_G | X86_MMU_PG_PS | X86_MMU_PG_RW | X86_MMU_PG_P | X86_MMU_PG_U)

#define PHYS(x) (x)


.section .text.boot0, "a", @progbits
DATA(_multiboot_header)
    /* magic */
    .int MULTIBOOT_HEADER_MAGIC
    /* flags */
    .int MULTIBOOT_HEADER_FLAGS
    /* checksum */
    .int MULTIBOOT_CHECKSUM
    /* header_addr */
    .int _multiboot_header
    /* load_addr */
    .int __code_start
    /* load_end_addr */
    .int __data_end
    /* bss_end_addr */
    .int _end
    /* entry_addr */
    .int _start
END_DATA(_multiboot_header)


/* shared code to set up a default 64bit page table structure */
.macro page_table_init
    /* Setting the First PML4E with a PDP table reference*/
    movl $PHYS(pdp), %eax
    orl  $X86_KERNEL_PD_FLAGS, %eax
    movl %eax, PHYS(pml4)

    /* Setting the First PDPTE with a Page table reference*/
    movl $PHYS(pte), %eax
    orl  $X86_KERNEL_PD_FLAGS, %eax
    movl %eax, PHYS(pdp)

    /* map the first 1GB in this table */
    movl $PHYS(pte), %esi
    movl $0x200, %ecx
    xor  %eax, %eax

0:
    mov  %eax, %ebx
    shll $21, %ebx
    orl  $X86_KERNEL_PD_LP_FLAGS, %ebx
    movl %ebx, (%esi)
    addl $8,%esi
    inc  %eax
    loop 0b
.endm

/* The magic number passed by a Multiboot-compliant boot loader. */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

.section .text.boot, "ax", @progbits
.code32
FUNCTION_LABEL(_start)
    cmpl $MULTIBOOT_BOOTLOADER_MAGIC, %eax
    je 0f
    xor %ebx,%ebx
0:  // multiboot_info_t pointer is in %ebx.

    // Load our new GDT by physical pointer.
    // _gdtr has it as a virtual pointer, so copy it and adjust.
    movw PHYS(_gdtr), %ax
    movl PHYS(_gdtr+2), %ecx
    sub $PHYS_ADDR_DELTA, %ecx
    movw %ax, -6(%esp)
    movl %ecx, -4(%esp)
    lgdt -6(%esp)

    /* load our data selectors */
    movw $DATA_SELECTOR, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* We need to jump to our sane 32 bit CS */
    pushl $CODE_SELECTOR
    pushl $PHYS(.Lfarjump)
    lret

.Lfarjump:
    /* zero the bss section */
    movl $PHYS(__bss_start), %edi
    movl $PHYS(_end), %ecx
    sub %edi, %ecx              // Compute the length of the bss in bytes.
    xor %eax, %eax
    rep stosb // while (ecx-- > 0) *edi++ = al;

    // _multiboot_info is in bss, so now it's safe to set it.
    movl %ebx, PHYS(_multiboot_info)

    /* Start using the zircon stack from its physical address. */
    mov $PHYS(_kstack_end), %esp

paging_setup:
    /* Preparing 64 bit paging, we will use 2MB pages covering 1GB
     * for initial bootstrap, this page table will be 1 to 1
     */

    /* Set the PAE bit to enable 64bit paging
     * Set PGE to enable global kernel pages
     */
    mov %cr4, %eax
    or $(X86_CR4_PAE|X86_CR4_PGE), %eax
    mov %eax, %cr4

    /* Long Mode Enabled at this point */
    movl $X86_MSR_IA32_EFER, %ecx
    rdmsr
    orl $X86_EFER_LME,%eax
    wrmsr

    /* initialize the default page tables */
    page_table_init

    /* load the physical pointer to the top level page table */
    movl $PHYS(pml4), %eax
    mov %eax, %cr3

    /* Enabling Paging and from this point we are in
    32 bit compatibility mode*/
    mov %cr0,  %eax
    btsl $(31), %eax
    mov %eax,  %cr0

    /* Using another long jump to be on 64 bit mode
    after this we will be on real 64 bit mode */
    pushl $CODE_64_SELECTOR     /*Need to put it in a the right CS*/
    pushl $PHYS(farjump64)
    lret

.align 8
.code64
farjump64:
    /* give the boot allocator a chance to compute the physical address of the kernel */
    //call boot_alloc_init

    /* branch to our high address */
    mov  $high_entry, %rax
    jmp  *%rax

high_entry:
    /* zero our kernel segment data registers */
    xor %eax, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss

    /* load the high kernel stack */
    mov  $_kstack_end, %rsp

    /* reload the gdtr */
    lgdt _gdtr

    // Set %gs.base to &bp_percpu.  It's statically initialized
    // with kernel_unsafe_sp set, so after this it's safe to call
    // into C code that might use safe-stack and/or stack-protector.
    //lea bp_percpu(%rip), %rax
    //mov %rax, %rdx
    //shr $32, %rdx
    //mov $X86_MSR_IA32_GS_BASE, %ecx
    //wrmsr

    /* set up the idt */
    //mov $_idt_startup, %rdi
    //call idt_setup
    //lidt _idtr

    /* assign this core CPU# 0 and initialize its per cpu state */
    //xor %edi, %edi
    //call x86_init_percpu

    // Fill the stack canary with a random value as early as possible.
    // This isn't done in x86_init_percpu because the hw_rng_get_entropy
    // call would make it eligible for stack-guard checking itself.  But
    // %gs is not set up yet in the prologue of the function, so it would
    // crash if it tried to use the stack-guard.
    //call choose_stack_guard

    // Move it into place.
    //mov %rcx, %gs:ZX_TLS_STACK_GUARD_OFFSET
    // Don't leak that value to other code.
    //xor %ecx, %ecx

    // configure the kernel base address
    // TODO: dynamically figure this out once we allow the x86 kernel to be loaded anywhere
    //movl $PHYS_LOAD_ADDRESS, kernel_base_phys(%rip)

    /* call the main module */
    call kernel_main

0:                          /* just sit around waiting for interrupts */
    hlt                     /* interrupts will unhalt the processor */
    pause
    jmp 0b                  /* so jump back to halt to conserve power */


.section .rodata

.align 8
DATA(_gdtr)
    .short _gdt_end - _gdt - 1
    .quad _gdt

.data
.align 8
DATA(_gdt)
    /* null entry */
    .int 0
    .int 0

    /* CODE_SELECTOR */
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b10011010       /* P(1) DPL(00) S(1) 1 C(0) R(1) A(0) */
    .byte  0b11001111       /* G(1) D(1) 0 0 limit 19:16 */
    .byte  0x0              /* base 31:24 */

    /* CODE_64_SELECTOR */
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b10011010       /* P(1) DPL(00) S(1) 1 C(0) R(1) A(0) */
    .byte  0b10101111       /* G(1) D(0) L(1) AVL(0) limit 19:16 */
    .byte  0x0              /* base 31:24 */

    /* DATA_SELECTOR */
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b10010010       /* P(1) DPL(00) S(1) 0 E(0) W(1) A(0) */
    .byte  0b11001111       /* G(1) B(1) 0 0 limit 19:16 */
    .byte  0x0              /* base 31:24 */

    /* disable 32bit ring3 code descriptor on 64bit kernel */
    .int  0x0
    .int  0x0

    /* USER_DATA_SELECTOR */
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b11110010       /* P(1) DPL(11) S(1) 0 E(0) W(1) A(0) */
    .byte  0b11001111       /* G(1) B(1) 0 0 limit 19:16 */
    .byte  0x0              /* base 31:24 */

    /* USER_CODE_64_SELECTOR */
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b11111010       /* P(1) DPL(11) S(1) 1 C(0) R(1) A(0) */
    .byte  0b10101111       /* G(1) D(0) L(1) AVL(0) limit 19:16 */
    .byte  0x0              /* base 31:24 */

    /* TSS_SELECTORs */
    .short 0                /* limit 15:00 */
    .short 0                /* base 15:00 */
    .byte  0                /* base 23:16 */
    .byte  0b10001001       /* P(1) DPL(00) 0 10 B(0) 1 */
    .byte  0b10000000       /* G(1) 0 0 AVL(0) limit 19:16 */
    .byte  0                /* base 31:24 */
    /* second half of 64bit desciptor */
    .int   0x00000000       /* base 63:32 */
    .int   0x00000000       /* reserved/sbz */

END_DATA(_gdt)

DATA(_gdt_end)

.bss
.align 16
DATA(_kstack)
    .skip 4096
DATA(_kstack_end)
