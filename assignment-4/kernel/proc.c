
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#define TINY 1

PUBLIC SEMAPHORE sems[MAX_SEM];
int sem_head;
int tiny_tick = TINY;
PUBLIC void wait_cur();
PUBLIC int queue_not_full(int head, int tail, int capacity);

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	for (PROCESS *p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
	{
		if (p->dly)
			p->dly--;
	}

	tiny_tick--;
	if (tiny_tick > 0)
		return;
	else
		tiny_tick = TINY;

	do
	{
		p_proc_ready = proc_table + (p_proc_ready - proc_table + 1) % (NR_TASKS + NR_PROCS);
	} while (p_proc_ready->dly || (p_proc_ready->stat & WAIT));
}

PUBLIC void wait_cur()
{
	p_proc_ready->stat |= WAIT;
}

PUBLIC int queue_not_full(int head, int tail, int capacity)
{
	return ((tail + 1) % capacity) != head;
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC int sys_print_str(const char *str, int color)
{
	TTY *p_tty = tty_table;
	int i = 0;
	while (str[i] != '\0')
	{
		out_colorful_char(p_tty->p_console, str[i], color);
		i++;
	}
	return 0;
}

PUBLIC int printn(int n)
{
	char num[40];
	char rev[40];
	int i = 0;
	do
	{
		rev[i++] = '0' + (n % 10);
		n /= 10;
	} while (n != 0);

	int l = i;
	while (i > 0)
	{
		num[l - i] = rev[i - 1];
		i--;
	}
	num[l] = '\0';

	write(num, l);
}

PUBLIC int sys_dly(int mills)
{
	p_proc_ready->dly = mills * HZ / 1000;
	schedule();
	return 0;
}

PUBLIC int sys_P(SEMAPHORE *t)
{
	disable_int();
	// print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
	// print_str(" P rmutex: ", DEFAULT_CHAR_COLOR);
	// printn(t->available);
	// print_str("\n", DEFAULT_CHAR_COLOR);
	t->available--;
	// print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
	// print_str(" after P rmutex: ", DEFAULT_CHAR_COLOR);
	// printn(t->available);
	// print_str("\n", DEFAULT_CHAR_COLOR);
	if (t->available < 0)
	{
		wait_cur();
		print_str(p_proc_ready->p_name, BLUE);
		print_str(" needs to wait\n", BLUE);
		if (queue_not_full(t->wait_head, t->wait_tail, MAX_WAITING))
			t->wait[t->wait_tail] = p_proc_ready; //进入等待进程队列
		t->wait_tail = (t->wait_tail + 1) % MAX_WAITING;
		enable_int();
		schedule();
	}
	else
	{
		enable_int();
	}
	return 0;
}

PUBLIC int sys_V(SEMAPHORE *t)
{
	disable_int();
	// print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
	// print_str(" V rmutex: ", DEFAULT_CHAR_COLOR);
	// printn(t->available);
	// print_str("\n", DEFAULT_CHAR_COLOR);
	t->available++;
	// print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
	// print_str(" after V rmutex: ", DEFAULT_CHAR_COLOR);
	// printn(t->available);
	// print_str("\n", DEFAULT_CHAR_COLOR);
	if (t->available <= 0)
	{
		t->wait[t->wait_head]->stat &= ~WAIT;
		print_str(t->wait[t->wait_head]->p_name, BLUE);
		print_str(" is released\n", BLUE);
		t->wait_head = (t->wait_head + 1) % MAX_WAITING;
	}
	enable_int();
	return 0;
}

PUBLIC SEMAPHORE *sem_create(int cnt)
{
	SEMAPHORE *sem = sems + sem_head++;
	sem->available = cnt;
	sem->wait_head = 0;
	sem->wait_tail = 0;
	return sem;
}
