
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#define DEBUG 0

int read_cnt;
int write_cnt;
int cur_proc_type;
SEMAPHORE *rmutex, *wmutex, *x, *y, *z, *read_available;
/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK *p_task = task_table;
	PROCESS *p_proc = proc_table;
	char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	u8 privilege;
	u8 rpl;
	int eflags;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if (i < NR_TASKS)
		{ /* 任务 */
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
		}
		else
		{ /* 用户进程 */
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202; /* IF=1, bit 2 is always 1 */
		}

		strcpy(p_proc->p_name, p_task->name); // name of the process
		p_proc->pid = i;					  // pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
			   sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
			   sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	x = sem_create(1);
	y = sem_create(1);
	z = sem_create(1);
	rmutex = sem_create(1);
	wmutex = sem_create(1);
	read_available = sem_create(READNUM);
	read_cnt = 0;
	write_cnt = 0;
	cur_proc_type = READING;

	init_clock();
	init_keyboard();

	restart();

	while (1)
	{
	}
}

void reader_A()
{
	dly(3000);
	reader(2);
}

void reader_B()
{
	dly(4000);
	reader(3);
}

void reader_C()
{
	dly(5000);
	reader(3);
}

void writer_D()
{
	dly(6000);
	writer(3);
}

void writer_E()
{
	dly(7000);
	writer(4);
}

int get_read_cnt()
{
	return read_cnt;
}

int get_cur_proc_type()
{
	return cur_proc_type;
}

#if DEBUG == 1
void reader(int i)
{
	while (1)
	{
		print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
		dly(i * TIMESLICE);
	}
}

void writer(int i)
{
	while (1)
	{
		print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
		dly(i * TIMESLICE);
	}
}

#else
void reader(int time)
{
	while (1)
	{
		P(read_available);
		int color = p_proc_ready->p_name[0] - 'A' + 1;
		// print_str(p_proc_ready->p_name, color);
		// print_str(" runs\n", color);
#if PRIORITY == WRITING
		P(z);
		P(rmutex);
		P(x);
		read_cnt++;
		if (read_cnt == 1)
			P(wmutex);
		V(x);
		V(rmutex);
		V(z);
#else
		P(rmutex);
		if (read_cnt == 0)
			P(wmutex);
		read_cnt++;
		V(rmutex);
#endif
		cur_proc_type = READING;
		print_str(p_proc_ready->p_name, color);
		print_str(" started reading\n", color);

		dly(time * TIMESLICE);

		print_str(p_proc_ready->p_name, color);
		print_str(" finished reading\n", color);

#if PRIORITY == WRITING
		P(x);
		read_cnt--;
		if (read_cnt == 0)
			V(wmutex);
		V(x);
#else
		P(rmutex);
		read_cnt--;
		if (read_cnt == 0)
			V(wmutex);
		V(rmutex);
#endif
		V(read_available);
	}
}

void writer(int time)
{
	while (1)
	{
#if PRIORITY == WRITING
		P(y);
		write_cnt++;
		if (write_cnt == 1)
			P(rmutex);
		V(y);
#endif
		int color = p_proc_ready->p_name[0] - 'A' + 1;
		P(wmutex);
		cur_proc_type = WRITING;
		print_str(p_proc_ready->p_name, color);
		print_str(" started writing\n", color);

		dly(time * TIMESLICE);

		print_str(p_proc_ready->p_name, color);
		print_str(" finished writing\n", color);
		V(wmutex);

#if PRIORITY == WRITING
		P(y);
		write_cnt--;
		if (write_cnt == 0)
			V(rmutex);
		V(y);
#endif
	}
}
#endif

void observer()
{
#if DEBUG == 1
	while (1)
	{
		print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
		dly(1 * TIMESLICE);
	}
#else
	while (1)
	{
		if (get_cur_proc_type() == READING)
		{
			print_str("READING: ", DEFAULT_CHAR_COLOR);
			printn(get_read_cnt());
			print_str(" readers are reading.\n", DEFAULT_CHAR_COLOR);
		}
		else
		{
			print_str("WRITING\n", DEFAULT_CHAR_COLOR);
		}
		dly(TIMESLICE);
	}
#endif
}
