/**
 * OS
 *
 * Priority-based real-time multitasking operating system
 *
 * @author Jeff Stubler
 * @date March 10, 2012
 */

#include "os.h"

typedef struct {
    char name[NAME_SIZE];
    uint32_t start_timestamp;
    uint16_t stack_pointer;
    uint8_t running;
    uint8_t delayed;
    uint8_t suspended;
    uint8_t semaphore_blocked;
} process_control_block;

static volatile uint8_t priority_buffer[NUMBER_OF_PROCESSES];
static process_control_block pcb[NUMBER_OF_PROCESSES];
static volatile uint8_t current_process;
static volatile uint16_t quantum_ticks = 0;
static volatile uint32_t system_ticks = 0;
static volatile uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE];

static void os_terminate_current_task(void) {
	os_remove_task(os_get_current_pid());
}

static void os_idle_task(void) {
	// Enable idle mode
	MCUCR |= (1 << SE);
	MCUCR &= 0xff - ((1 << SM2) | (1 << SM1) | (1 << SM0));
	while (1) {
		asm("sleep");
	}
}

void enable_timer(void) {
	TCNT0 = 0;
	TCCR0 = (1 << WGM01) | (1 << CS01) | (1 << CS00); /// CTC mode, clk/64
	OCR0 = 250; // clk/64/250 = clk/16000
	TIMSK |= (1 << OCIE0);
}

static void os_choose_next_process(void) {
    int current_priority;
	for (current_priority = 0; current_priority < NUMBER_OF_PROCESSES; current_priority++) {
		uint8_t selected_pid = priority_buffer[current_priority];
		if (selected_pid == 0xff) {
			continue;
		} else {
			if (pcb[selected_pid].running == 1 && pcb[selected_pid].delayed == 0 && pcb[selected_pid].suspended == 0 && pcb[selected_pid].semaphore_blocked == 0) {
				current_process = selected_pid;
				break;
			// TODO: WARNING: After 49 days at 1 kHz system tick increment, delays will not work as system_ticks will have rolled over
			} else if (pcb[selected_pid].delayed == 1 && system_ticks >= pcb[selected_pid].start_timestamp) {
				pcb[selected_pid].delayed = 0;
				current_process = selected_pid;
				break;
			}
		}
	}
}

NAKED_FUNCTION(schedule) {
	asm volatile("push r0\n");
	asm volatile("in r0, __SREG__\n");
	asm volatile("cli\n");
	asm volatile("push r0\n");
	asm volatile("push r1\n");
	asm volatile("clr r1\n");
	asm volatile("push r2\n");
	asm volatile("push r3\n");
	asm volatile("push r4\n");
	asm volatile("push r5\n");
	asm volatile("push r6\n");
	asm volatile("push r7\n");
	asm volatile("push r8\n");
	asm volatile("push r9\n");
	asm volatile("push r10\n");
	asm volatile("push r11\n");
	asm volatile("push r12\n");
	asm volatile("push r13\n");
	asm volatile("push r14\n");
	asm volatile("push r15\n");
	asm volatile("push r16\n");
	asm volatile("push r17\n");
	asm volatile("push r18\n");
	asm volatile("push r19\n");
	asm volatile("push r20\n");
	asm volatile("push r21\n");
	asm volatile("push r22\n");
	asm volatile("push r23\n");
	asm volatile("push r24\n");
	asm volatile("push r25\n");
	asm volatile("push r26\n");
	asm volatile("push r27\n");
	asm volatile("push r28\n");
	asm volatile("push r29\n");
	asm volatile("push r30\n");
	asm volatile("push r31\n");

	pcb[current_process].stack_pointer = STACK_HIGH << 8 | STACK_LOW;
	// TODO: Investigate round robbin of different tasks at priority level, ready list for quick context switcher

	/*do {
		current_process = (current_process + 1) % NUMBER_OF_PROCESSES;
	} while (pcb[current_process].state == OS_UNINITIALIZED_STATE || pcb[current_process].delay_ticks > 0);*/
	os_choose_next_process();

	STACK_HIGH = (uint8_t) (pcb[current_process].stack_pointer >> 8);
	STACK_LOW = (uint8_t) (pcb[current_process].stack_pointer & 0xff);

	asm volatile("pop r31\n");
	asm volatile("pop r30\n");
	asm volatile("pop r29\n");
	asm volatile("pop r28\n");
	asm volatile("pop r27\n");
	asm volatile("pop r26\n");
	asm volatile("pop r25\n");
	asm volatile("pop r24\n");
	asm volatile("pop r23\n");
	asm volatile("pop r22\n");
	asm volatile("pop r21\n");
	asm volatile("pop r20\n");
	asm volatile("pop r19\n");
	asm volatile("pop r18\n");
	asm volatile("pop r17\n");
	asm volatile("pop r16\n");
	asm volatile("pop r15\n");
	asm volatile("pop r14\n");
	asm volatile("pop r13\n");
	asm volatile("pop r12\n");
	asm volatile("pop r11\n");
	asm volatile("pop r10\n");
	asm volatile("pop r9\n");
	asm volatile("pop r8\n");
	asm volatile("pop r7\n");
	asm volatile("pop r6\n");
	asm volatile("pop r5\n");
	asm volatile("pop r4\n");
	asm volatile("pop r3\n");
	asm volatile("pop r2\n");
	asm volatile("pop r1\n");
	asm volatile("pop r0\n");
	asm volatile("out __SREG__, r0\n");
	asm volatile("pop r0\n");
	asm volatile("ret");
}

/**
 * Initialize operating system
 */
void os_init(void) {
    uint8_t pcb_index;
	for (pcb_index = 0; pcb_index < NUMBER_OF_PROCESSES; pcb_index++) {
		pcb[pcb_index].running = 0;
        pcb[pcb_index].delayed = 0;
        pcb[pcb_index].suspended = 0;
        pcb[pcb_index].semaphore_blocked = 0;
		copy_string(pcb[pcb_index].name, NAME_SIZE, "");
		pcb[pcb_index].stack_pointer = 0;
		priority_buffer[pcb_index] = 0xff;
	}

	pcb[0].running = 1;
	copy_string(pcb[0].name, NAME_SIZE, "init");
	pcb[0].stack_pointer = STACK_HIGH << 8 | STACK_LOW;
	priority_buffer[NUMBER_OF_PROCESSES - 2] = 0;

	current_process = 0;

	os_add_task(os_idle_task, &idle_task_stack[IDLE_TASK_STACK_SIZE - 1], NUMBER_OF_PROCESSES - 1, "idle");

	enable_timer();
}

/**
 * Start multitasking with the timer ticker
 */
void os_start_ticker(void) {
	asm("sei");
}

/**
 * Delay task for specified number of ticks
 *
 * @param pid Process ID to delay
 * @param ticks Number of ticks to delay
 */
int8_t os_delay(uint8_t pid, uint32_t ticks) {
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	uint32_t start_timestamp = system_ticks + ticks;
	ENTER_CRITICAL_SECTION();
	pcb[pid].start_timestamp = start_timestamp;
    pcb[pid].delayed = 1;
	LEAVE_CRITICAL_SECTION();
	schedule();
	return 0;
}

/**
 * Cancel any delay on a task
 * @param pid Process ID to cancel delay for
 */
int8_t os_cancel_delay(uint8_t pid) {
    if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
        return -1;
    }
    ENTER_CRITICAL_SECTION();
    pcb[pid].delayed = 0;
    LEAVE_CRITICAL_SECTION();
    schedule();
    return 0;
}

/**
 * Add new task to operating system
 */
int8_t os_add_task(void (*task)(void), volatile uint8_t *stack, uint8_t priority, char *name) {
	if (priority < 0 || priority >= NUMBER_OF_PROCESSES) {
		return -1;
	}

	ENTER_CRITICAL_SECTION();

	uint8_t current_pcb = 0;

	while (pcb[current_pcb].running == 1) {
		current_pcb++;
	}

	if (priority_buffer[priority] != 0xff || current_pcb >= NUMBER_OF_PROCESSES) {
		LEAVE_CRITICAL_SECTION();
		return -1;
	}

	pcb[current_pcb].running = 1;
	copy_string(pcb[current_pcb].name, NAME_SIZE, name);
	pcb[current_pcb].stack_pointer = (uint16_t) stack;

	// When process returns, call void function to remove process
	*(uint8_t *)pcb[current_pcb].stack_pointer = (uint8_t) ((uint16_t) os_terminate_current_task & 0xff);
	pcb[current_pcb].stack_pointer--;
	*(uint8_t *)pcb[current_pcb].stack_pointer = (uint8_t) ((uint16_t) os_terminate_current_task >> 8);
	pcb[current_pcb].stack_pointer--;

	// Add process start address to stack to be popped off
	*(uint8_t *) pcb[current_pcb].stack_pointer = (uint8_t) ((uint16_t) task & 0xff);
	pcb[current_pcb].stack_pointer--;
	*(uint8_t *) pcb[current_pcb].stack_pointer = (uint8_t) ((uint16_t) task >> 8);
	pcb[current_pcb].stack_pointer--;

	// Register 0
	*(uint8_t *) pcb[current_pcb].stack_pointer = 0;
	pcb[current_pcb].stack_pointer--;
	// SREG, new task starts with interrupts enabled
	*(uint8_t *) pcb[current_pcb].stack_pointer = 0x80;
	pcb[current_pcb].stack_pointer--;

	// Register 1 to Register 31
	pcb[current_pcb].stack_pointer -= 31;

	priority_buffer[priority] = current_pcb;

	LEAVE_CRITICAL_SECTION();

	return current_pcb;
}

/**
 * Remove a task from running
 */
int8_t os_remove_task(uint8_t pid) {
	int current_priority;
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].running = 0;
	for (current_priority = 0; current_priority < NUMBER_OF_PROCESSES; current_priority++) {
		if (priority_buffer[current_priority] == pid) {
			priority_buffer[current_priority] = 0xff;
		}
	}
	LEAVE_CRITICAL_SECTION();
	schedule();
	return 0;
}

/**
 * Get current task
 */
uint8_t os_get_current_pid(void) {
	return current_process;
}

// TODO: Add destination buffer size and do not overwrite it
void copy_string(char *destination, uint8_t destination_size, char *source) {
	char *original_destination = destination;
	uint8_t original_destination_size = destination_size;
	while (*source != '\0' && destination_size > 0) {
		*destination = *source;
		destination++;
		source++;
		destination_size--;
	}
	original_destination[original_destination_size - 1] = '\0';
}

void os_set_task_name(uint8_t pid, char *name) {
	copy_string(pcb[pid].name, NAME_SIZE, name);
}

char *os_get_task_name(uint8_t pid) {
	return pcb[pid].name;
}

int8_t os_set_task_priority(uint8_t pid, uint8_t priority) {
	if (priority < 0 || priority >= NUMBER_OF_PROCESSES || pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	uint8_t old_priority = os_get_task_priority(pid);
	if (priority_buffer[priority] == 0xff) {
		priority_buffer[priority] = pid;
		priority_buffer[old_priority] = 0xff;
	}
	LEAVE_CRITICAL_SECTION();
	schedule();
	return 0;
}

int8_t os_get_task_priority(uint8_t pid) {
	int current_priority;
	for (current_priority = 0; current_priority < NUMBER_OF_PROCESSES; current_priority++) {
		if (priority_buffer[current_priority] == pid) {
			return current_priority;
		}
	}
	return -1;
}

int8_t os_suspend_task(uint8_t pid) {
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].suspended = 1;
	LEAVE_CRITICAL_SECTION();
	schedule();
	return 0;
}

int8_t os_resume_task(uint8_t pid) {
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].suspended = 0;
	LEAVE_CRITICAL_SECTION();
	return 0;
}

void os_semaphore_init(os_semaphore *semaphore, uint8_t count) {
    semaphore->count = count;
    uint8_t pid;
    for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
        semaphore->wait_list[pid] = 0;
    }
}

int8_t os_semaphore_wait(os_semaphore *semaphore) {
    ENTER_CRITICAL_SECTION();
    if (semaphore->count > 0) {
        semaphore->count--;
        LEAVE_CRITICAL_SECTION();
        return 0;
    }
    uint8_t pid = os_get_current_pid();
    semaphore->wait_list[pid] = 1;
    pcb[pid].semaphore_blocked = 1;
    LEAVE_CRITICAL_SECTION();
    schedule();
    ENTER_CRITICAL_SECTION_AGAIN();
    semaphore->count--;
    LEAVE_CRITICAL_SECTION();
    return 0;
}

int8_t os_semaphore_signal(os_semaphore *semaphore) {
        ENTER_CRITICAL_SECTION();
        if (semaphore->count == 0) {
            uint8_t pid;
            for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
                pcb[pid].semaphore_blocked = 0;
                semaphore->wait_list[pid] = 0;
            }
            LEAVE_CRITICAL_SECTION();
            schedule();
            ENTER_CRITICAL_SECTION_AGAIN();
        }
        if (semaphore->count < 255) {
            semaphore->count++;
            LEAVE_CRITICAL_SECTION();
            return 0;
        }
        LEAVE_CRITICAL_SECTION();
        return -1;
}

ISR(TIMER0_COMP_vect) {
	quantum_ticks++;
	system_ticks++;

	if (quantum_ticks >= QUANTUM_MILLISECOND_LENGTH) {
		quantum_ticks = 0;
		schedule();
	}
}
