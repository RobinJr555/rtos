/*
 * (C) Copyright 2017
 * Mingjun Zhu, <zhumj1991@163.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

	.syntax	unified
	.arch	armv7-m

/*
 * Memory Model
 * The HEAP starts at the end of the DATA section and grows upward.
 *
 * The STACK starts at the end of the RAM and grows downward.
 *
 * The HEAP and stack STACK are only checked at compile time:
 * (DATA_SIZE + HEAP_SIZE + STACK_SIZE) < RAM_SIZE
 *
 * This is just a check for the bare minimum for the Heap+Stack area before
 * aborting compilation, it is not the run time limit:
 * Heap_Size + Stack_Size = 0x80 + 0x80 = 0x100
 */
	.section .stack
	.align 3
#ifdef __STACK_SIZE
	.equ	Stack_Size, __STACK_SIZE
#else
	.equ	Stack_Size, 0x400
#endif
	.globl	__StackTop
	.globl	__StackLimit
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, . - __StackTop

	.section .heap
	.align 3
#ifdef __HEAP_SIZE
	.equ	Heap_Size, __HEAP_SIZE
#else
	.equ	Heap_Size, 0xC00
#endif
	.globl	__HeapBase
	.globl	__HeapLimit
__HeapBase:
	.space	Heap_Size
	.size	__HeapBase, . - __HeapBase
__HeapLimit:
	.size	__HeapLimit, . - __HeapLimit

	.section .vector_table, "a", %progbits
	.align 2
	.globl __isr_vector
__isr_vector:
	.long	__StackTop            /* Top of Stack */
	.long	Reset_Handler         /* Reset Handler */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	MemManage_Handler     /* MPU Fault Handler */
	.long	BusFault_Handler      /* Bus Fault Handler */
	.long	UsageFault_Handler    /* Usage Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_Handler           /* SVCall Handler */
	.long	DebugMon_Handler      /* Debug Monitor Handler */
	.long	0                     /* Reserved */
	.long	PendSV_Handler        /* PendSV Handler */
	.long	SysTick_Handler       /* SysTick Handler */

	/* External interrupts */
	.long	SYSTEM_ON_IRQHandler
	.long	WDG_IRQHandler
	.long	TIMER0_IRQHandler
	.long	TIMER1_IRQHandler
	.long	I2C3_IRQHandler
	.long	TIMER2_7_IRQHandler
	.long	SPI0_IRQHandler
	.long	GPIO_IRQHandler
	.long	UART0_IRQHandler
	.long	SPI_FLASH_IRQHandler
	.long	USB_OTG_IRQHandler
	.long	SDIO_HOST_IRQHandler
	.long	SDIO_DEVICE_IRQHandler
	.long	I2S0_PCM0_IRQHandler
	.long	I2S1_PCM1_IRQHandler
	.long	WL_DMA_IRQHandler
	.long	WLXXX1_IRQHandler
	.long	WL_PROTOCOL_IRQHandler
	.long	WLXXX3_IRQHandler
	.long	PERIPHERAL_IRQHandler
	.long	GDMA0_CHANNEL0_IRQHandler
	.long	GDMA0_CHANNEL1_IRQHandler
	.long	GDMA0_CHANNEL2_IRQHandler
	.long	GDMA0_CHANNEL3_IRQHandler
	.long	GDMA0_CHANNEL4_IRQHandler
	.long	GDMA0_CHANNEL5_IRQHandler
	.long	GDMA1_CHANNEL0_IRQHandler
	.long	GDMA1_CHANNEL1_IRQHandler
	.long	GDMA1_CHANNEL2_IRQHandler
	.long	GDMA1_CHANNEL3_IRQHandler
	.long	GDMA1_CHANNEL4_IRQHandler
	.long	GDMA1_CHANNEL5_IRQHandler

	.size	__isr_vector, . - __isr_vector

	.section .text.Reset_Handler
	.thumb
	.thumb_func
	.align 2
	.globl	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
	/* FIXME: refine a complete startup */
	push	{r0, lr}
	ldr		r0, =SystemInit
	blx		r0
	ldr		r0, =_start
	bx		r0
	.pool
	.size Reset_Handler, . - Reset_Handler

/*
 * Macro to define default handlers. Default handler
 * will be weak symbol and just dead loops. They can be
 * overwritten by other handlers.
 */
	.text
	.macro	def_default_handler		handler_name
	.align 1
	.thumb_func
	.weak	\handler_name
	.type	\handler_name, %function
\handler_name:
	b		.
	.size	\handler_name, . - \handler_name
	.endm

	def_default_handler		NMI_Handler
	def_default_handler		HardFault_Handler
	def_default_handler		SVC_Handler
	def_default_handler		PendSV_Handler
	def_default_handler		SysTick_Handler
	def_default_handler		Default_Handler

	.macro	def_fault_default_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

	def_fault_default_handler	MemManage_Handler
	def_fault_default_handler	BusFault_Handler
	def_fault_default_handler	UsageFault_Handler
	def_fault_default_handler	DebugMon_Handler

	.macro	def_irq_default_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

	def_irq_default_handler		SYSTEM_ON_IRQHandler
	def_irq_default_handler		WDG_IRQHandler
	def_irq_default_handler		TIMER0_IRQHandler
	def_irq_default_handler		TIMER1_IRQHandler
	def_irq_default_handler		I2C3_IRQHandler
	def_irq_default_handler		TIMER2_7_IRQHandler
	def_irq_default_handler		SPI0_IRQHandler
	def_irq_default_handler		GPIO_IRQHandler
	def_irq_default_handler		UART0_IRQHandler
	def_irq_default_handler		SPI_FLASH_IRQHandler
	def_irq_default_handler		USB_OTG_IRQHandler
	def_irq_default_handler		SDIO_HOST_IRQHandler
	def_irq_default_handler		SDIO_DEVICE_IRQHandler
	def_irq_default_handler		I2S0_PCM0_IRQHandler
	def_irq_default_handler		I2S1_PCM1_IRQHandler
	def_irq_default_handler		WL_DMA_IRQHandler
	def_irq_default_handler		WLXXX1_IRQHandler
	def_irq_default_handler		WL_PROTOCOL_IRQHandler
	def_irq_default_handler		WLXXX3_IRQHandler
	def_irq_default_handler		PERIPHERAL_IRQHandler
	def_irq_default_handler		GDMA0_CHANNEL0_IRQHandler
	def_irq_default_handler		GDMA0_CHANNEL1_IRQHandler
	def_irq_default_handler		GDMA0_CHANNEL2_IRQHandler
	def_irq_default_handler		GDMA0_CHANNEL3_IRQHandler
	def_irq_default_handler		GDMA0_CHANNEL4_IRQHandler
	def_irq_default_handler		GDMA0_CHANNEL5_IRQHandler
	def_irq_default_handler		GDMA1_CHANNEL0_IRQHandler
	def_irq_default_handler		GDMA1_CHANNEL1_IRQHandler
	def_irq_default_handler		GDMA1_CHANNEL2_IRQHandler
	def_irq_default_handler		GDMA1_CHANNEL3_IRQHandler
	def_irq_default_handler		GDMA1_CHANNEL4_IRQHandler
	def_irq_default_handler		GDMA1_CHANNEL5_IRQHandler
