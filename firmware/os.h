/**
 * OS
 *
 * Priority-based soft real-time multitasking operating system
 *
 * @author Jeff Stubler
 * @date March 10, 2012
 */

#ifndef OS
#define OS

#include <inttypes.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>

//#include "atmega32.h"

// TODO: Add preprocess check on various params
/**
 * Maximum number of processes that can be managed
 */
#define NUMBER_OF_PROCESSES 6

/**
 * Length of each time quantum (ms)
 */
#define QUANTUM_MILLISECOND_LENGTH 10

#define NAME_SIZE 5

/**
 * Stack size
 */
#define IDLE_TASK_STACK_SIZE 64

/**
 * Semaphore structure
 */

typedef struct {
    uint8_t count;
    uint8_t wait_list[NUMBER_OF_PROCESSES];
} os_semaphore;

/* Naked functions and interrupts */

#define NAKED_ISR(vector) \
	void vector(void) __attribute__ ((signal, naked, __INTR_ATTRS)); \
	void vector(void)

#define NAKED_FUNCTION(function) \
	void function(void) __attribute__ ((naked, noinline)); \
	void function(void)

/* Clothing for naked functions */

#define SAVE_CONTEXT() \
	asm volatile ( \
		"push r0 \n\t" \
		"in r0, __SREG__ \n\t" \
		"cli \n\t" \
		"push r0 \n\t" \
		"push r1 \n\t" \
		"clr r1 \n\t" \
		"push r2 \n\t" \
		"push r3 \n\t" \
		"push r4 \n\t" \
		"push r5 \n\t" \
		"push r6 \n\t" \
		"push r7 \n\t" \
		"push r8 \n\t" \
		"push r9 \n\t" \
		"push r10 \n\t" \
		"push r11 \n\t" \
		"push r12 \n\t" \
		"push r13 \n\t" \
		"push r14 \n\t" \
		"push r15 \n\t" \
		"push r16 \n\t" \
		"push r17 \n\t" \
		"push r18 \n\t" \
		"push r19 \n\t" \
		"push r20 \n\t" \
		"push r21 \n\t" \
		"push r22 \n\t" \
		"push r23 \n\t" \
		"push r24 \n\t" \
		"push r25 \n\t" \
		"push r26 \n\t" \
		"push r27 \n\t" \
		"push r28 \n\t" \
		"push r29 \n\t" \
		"push r30 \n\t" \
		"push r31 \n\t" \
	)

#define RESTORE_CONTEXT() \
	asm volatile ( \
		"pop r31 \n\t" \
		"pop r30 \n\t" \
		"pop r29 \n\t" \
		"pop r28 \n\t" \
		"pop r27 \n\t" \
		"pop r26 \n\t" \
		"pop r25 \n\t" \
		"pop r24 \n\t" \
		"pop r23 \n\t" \
		"pop r22 \n\t" \
		"pop r21 \n\t" \
		"pop r20 \n\t" \
		"pop r19 \n\t" \
		"pop r18 \n\t" \
		"pop r17 \n\t" \
		"pop r16 \n\t" \
		"pop r15 \n\t" \
		"pop r14 \n\t" \
		"pop r13 \n\t" \
		"pop r12 \n\t" \
		"pop r11 \n\t" \
		"pop r10 \n\t" \
		"pop r9 \n\t" \
		"pop r8 \n\t" \
		"pop r7 \n\t" \
		"pop r6 \n\t" \
		"pop r5 \n\t" \
		"pop r4 \n\t" \
		"pop r3 \n\t" \
		"pop r2 \n\t" \
		"pop r1 \n\t" \
		"pop r0 \n\t" \
		"out __SREG__, r0 \n\t" \
		"pop r0 \n\t" \
	)

#define LEAVE_NAKED_ISR() asm("reti");
#define LEAVE_NAKED_FUNCTION() asm("reti");

/* Critical sections */

#define ENTER_CRITICAL_SECTION() \
	uint8_t flags = SREG; \
	asm volatile ("cli");

#define ENTER_CRITICAL_SECTION_AGAIN() \
    flags = SREG; \
    asm volatile ("cli");
	
#define LEAVE_CRITICAL_SECTION() \
	SREG = flags;

/* Stack */

#define STACK_HIGH (*((volatile uint8_t *)(0x5e)))
#define STACK_LOW (*((volatile uint8_t *)(0x5d)))

/* Memory information */

#define TOP_OF_MEMORY 0x085f

/**
 * Initialize operating system
 */
void os_init(void);

/**
 * Start multitasking with the timer ticker
 */
void os_start_ticker(void);

/**
 * Add new task to operating system
 */
int8_t os_add_task(void (*task)(void), volatile uint8_t *stack, uint8_t priority, char *name);

/**
 * Remove a task from the operating system
 */
int8_t os_remove_task(uint8_t pid);

/**
 * Get current task
 */
uint8_t os_get_current_pid(void);

void copy_string(char *destination, uint8_t destination_size, char *source);

/**
 * Delay task for specified number of ticks
 *
 * @param pid Process ID to delay
 * @param ticks Number of ticks to delay
 */
int8_t os_delay(uint8_t pid, uint32_t ticks);

/**
 * Cancel any delay on a task
 * @param pid Process ID to cancel delay for
 * @return Error code
 */
int8_t os_cancel_delay(uint8_t pid);

void os_set_task_name(uint8_t pid, char *name);
char *os_get_task_name(uint8_t pid);

int8_t os_set_task_priority(uint8_t pid, uint8_t priority);
int8_t os_get_task_priority(uint8_t pid);

int8_t os_suspend_task(uint8_t pid);
int8_t os_resume_task(uint8_t pid);

void os_semaphore_init(os_semaphore *semaphore, uint8_t count);
int8_t os_semaphore_wait(os_semaphore *semaphore);
int8_t os_semaphore_signal(os_semaphore *semaphore);

#endif
