
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

SEMAPHORE *rc_lock;
SEMAPHORE *wc_lock;
SEMAPHORE *rmutex;
SEMAPHORE *wmutex;
SEMAPHORE *ravailable;
int read_cnt = 0;
int write_cnt = 0;
char *readers[READER_NUM];
char *writers[1];
int proc_slices[NR_TASKS + NR_PROCS] = {0, 1, 3, 3, 3, 4, 1};

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

	init_memory();

	proc_table[0].priority = 3;
	proc_table[1].priority = 2;
	proc_table[2].priority = 2;
	proc_table[3].priority = 2;
	proc_table[4].priority = 2;
	proc_table[5].priority = 2;
	proc_table[6].priority = 1;

	proc_table[0].ticks = proc_table[0].req_ticks = 10;
	proc_table[1].ticks = proc_table[1].req_ticks = 1 * TIMESLICE;
	proc_table[2].ticks = proc_table[2].req_ticks = 3 * TIMESLICE;
	proc_table[3].ticks = proc_table[3].req_ticks = 3 * TIMESLICE;
	proc_table[4].ticks = proc_table[4].req_ticks = 3 * TIMESLICE;
	proc_table[5].ticks = proc_table[5].req_ticks = 4 * TIMESLICE;
	proc_table[6].ticks = proc_table[6].req_ticks = 1 * TIMESLICE;

	for (i = 1; i < NR_TASKS + NR_PROCS; i++)
	{
		// proc_table[i].ticks = (proc_table[i].req_ticks = (proc_slices[i] * TIMESLICE));
		ready(proc_table + i);
	}

	// proc_table[6].ticks = proc_table[6].req_ticks = 100;

	// proc_table[1].nr_tty = 0;
	// proc_table[2].nr_tty = 1;
	// proc_table[3].nr_tty = 1;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	rc_lock = sem_create(1);
	wc_lock = sem_create(1);
	rmutex = sem_create(1);
	wmutex = sem_create(1);
	ravailable = sem_create(READNUM);
	for (i = 0; i < 3; i++)
	{
		readers[i] = 0;
	}
	writers[0] = 0;

	init_clock();
	init_keyboard();

	restart();

	while (1)
	{
	}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
	while (1)
	{
		printf("<Ticks:%x>", get_ticks());
		milli_delay(200);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while (1)
	{
		printf("B");
		milli_delay(200);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
	while (1)
	{
		printf("C");
		milli_delay(200);
	}
}

void reader_a1()
{
	while (1)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str("ticks: ", BLUE);
		printn(p_proc_ready->ticks, BLUE);
		print_str("A", DEFAULT_CHAR_COLOR);
		// print_str("\n", DEFAULT_CHAR_COLOR);
		mock_run();
		// int idx = 0;
		// for (; idx < 3; idx++)
		// 	if (readers[idx] == 0)
		// 		break;

		// int color = p_proc_ready->p_name[0] - 'A' + 1;
		// P(ravailable);
		// readers[idx] = p_proc_ready->p_name;

		// print_str(p_proc_ready->p_name, color);
		// print_str(" starts reading\n", color);
		// P(rc_lock);
		// if (read_cnt == 0)
		// 	P(wc_lock);
		// read_cnt++;
		// V(rc_lock);
		// print_str(p_proc_ready->p_name, color);
		// print_str(" is reading...\n", color);
		// mock_run(proc_slices[p_proc_ready->p_name[0] - 'A'] * TIMESLICE);
		// P(rc_lock);
		// read_cnt--;
		// if (read_cnt == 0)
		// 	V(wc_lock);
		// V(rc_lock);
		// print_str(p_proc_ready->p_name, color);
		// print_str(" ends reading\n", color);
		// readers[idx] = 0;
		// V(ravailable);
	}
}

void reader_b1()
{
	while (1)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str("ticks: ", BLUE);
		printn(p_proc_ready->ticks, BLUE);
		print_str("B", DEFAULT_CHAR_COLOR);
		mock_run();
	}
}

void reader_c1()
{
	while (1)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str("ticks: ", BLUE);
		printn(p_proc_ready->ticks, BLUE);
		print_str("C", DEFAULT_CHAR_COLOR);
		mock_run();
	}
}

void writer_d1()
{
	while (1)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str("ticks: ", BLUE);
		printn(p_proc_ready->ticks, BLUE);
		print_str("D", DEFAULT_CHAR_COLOR);
		// print_str("\n", DEFAULT_CHAR_COLOR);
		mock_run();
		// writers[1] = p_proc_ready->p_name;
		// P(wc_lock);
		// int color = p_proc_ready->p_name[0] - 'A' + 1;
		// print_str(p_proc_ready->p_name, color);
		// print_str(" starts writing\n", color);
		// print_str(p_proc_ready->p_name, color);
		// print_str(" is writing...\n", color);
		// mock_run(proc_slices[p_proc_ready->p_name[0] - 'A'] * TIMESLICE);
		// print_str(p_proc_ready->p_name, color);
		// print_str(" ends reading\n", color);
		// writers[1] = 0;
		// V(wc_lock);
	}
}

void writer_e1()
{
	while (1)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str("ticks: ", BLUE);
		printn(p_proc_ready->ticks, BLUE);
		print_str("E", DEFAULT_CHAR_COLOR);
		// print_str("\n", DEFAULT_CHAR_COLOR);
		mock_run();
	}
}

void reader_2()
{
	while (1)
	{
		int color = p_proc_ready->p_name[0] - 'A' + 1;
		P(ravailable);

		P(rmutex);
		print_str(p_proc_ready->p_name, color);
		print_str(" starts reading\n", color);

		P(rc_lock);
		read_cnt++;
		if (read_cnt == 1)
			P(wmutex);
		V(rc_lock);
		V(rmutex);

		print_str(p_proc_ready->p_name, color);
		print_str(" is reading...\n", color);
		mock_run(proc_slices[p_proc_ready->p_name[0] - 'A'] * TIMESLICE);

		P(rc_lock);
		read_cnt--;
		if (read_cnt == 0)
			V(wmutex);
		V(rc_lock);
		print_str(p_proc_ready->p_name, color);
		print_str(" ends reading\n", color);

		V(ravailable);
	}
}

void writer_2()
{
	while (1)
	{
		int color = p_proc_ready->p_name[0] - 'A' + 1;
		P(wc_lock);
		write_cnt++;
		if (write_cnt == 1)
			P(rmutex);
		V(wc_lock);

		P(wmutex);
		print_str(p_proc_ready->p_name, color);
		print_str(" starts writing\n", color);
		print_str(p_proc_ready->p_name, color);
		print_str(" is writing\n", color);
		mock_run(proc_slices[p_proc_ready->p_name[0] - 'A'] * TIMESLICE);
		print_str(p_proc_ready->p_name, color);
		print_str(" ends reading\n", color);
		V(wmutex);

		P(wc_lock);
		write_cnt--;
		if (write_cnt == 0)
			V(rmutex);
		V(wc_lock);
	}
}

void observer()
{
	while (1)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str(" start, ticks: ", BLUE);
		printn(p_proc_ready->ticks, BLUE);
		print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
		// print_str("??", DEFAULT_CHAR_COLOR);
		// print_str("\n", DEFAULT_CHAR_COLOR);
		// if (read_cnt > 0)
		// {
		// 	int c = 0;
		// 	for (int i = 0; i < 3; i++)
		// 	{
		// 		if (readers[i] == 0)
		// 			break;
		// 		c++;
		// 		print_str("Process ", WHITE);
		// 		print_str(readers[i], WHITE);
		// 		print_str("is reading\n", WHITE);
		// 	}
		// 	char res[40];
		// 	vsprintf(res, "Total number of readers is %d\n", c);
		// 	print_str(res, WHITE);
		// }
		// else if (write_cnt > 0)
		// {
		// 	if (writers[0] != 0)
		// 	{
		// 		print_str("Process ", WHITE);
		// 		print_str(writers[0], WHITE);
		// 		print_str("is writing\n", WHITE);
		// 	}
		// }

		mock_run();
		print_str("finish2", DEFAULT_CHAR_COLOR);
	}
}
