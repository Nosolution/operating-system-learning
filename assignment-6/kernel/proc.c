
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

PUBLIC SEMAPHORE sems[MAX_SEM];
int sem_head = 0;

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	for (PROCESS *p = proc_table; p < proc_table + NR_TASKS; p++)
	{
		if (p->dly)
			p->dly--;
	}

	do
	{
		p_proc_ready = proc_table + (p_proc_ready - proc_table + 1) % NR_TASKS;
	} while (p_proc_ready->dly || (p_proc_ready->stat & WAIT));

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

PUBLIC int sys_print_str(char *str, int color)
{
	TTY *p_tty = tty_table;
	int i = 0;
	int z = p_proc_ready - proc_table - 3;
	while (str[i] != '\0')
	{
		out_colorful_char(p_tty->p_console, str[i], color);
		i++;
	}
}

PUBLIC int sys_dly(int mills)
{
	p_proc_ready->dly = mills * HZ / 1000;
	schedule();
	return 0;
}

PUBLIC int sys_P(SEMAPHORE *t)
{

	t->available--;
	if (t->available < 0)
	{
		if (queue_not_full(t->wait_head, t->wait_tail, MAX_WAITING))
			t->wait[t->wait_tail] = p_proc_ready; //进入等待进程队列
		t->wait_tail = (t->wait_tail + 1) % MAX_WAITING;
		schedule();
	}
	return 0;
}

PUBLIC int sys_V(SEMAPHORE *t)
{
	t->available++;
	if (t->available <= 0)
	{
		t->wait[t->wait_head]->stat &= ~WAIT;
		t->wait_head = (t->wait_head + 1) % MAX_WAITING;
	}
	return 0;
}

PUBLIC SEMAPHORE *sem_create(int cnt)
{
	sems[sem_head].available = cnt;
	return sems + (sem_head++);
}

