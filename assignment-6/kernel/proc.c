
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

/*======================================================================*
                              schedule
 *======================================================================*/
// SEMAPHORE semaphore1;
// SEMAPHORE semaphore2;
// SEMAPHORE semaphore3;
SEMAPHORE *sp1;
SEMAPHORE *sp2;
SEMAPHORE *sp3;
char *cname = "1";

PUBLIC SEMAPHORE sems[MAX_SEM];
int sem_head;

// PUBLIC SEMAPHORE *sem_create(int cnt);
PUBLIC void schedule()
{
	PROCESS *p;
	int greatest_ticks = 0;

	for (p = proc_table; p < proc_table + NR_TASKS; p++)
	{
		if (p->dly > 0)
			p->dly--;
	}
	while (!greatest_ticks)
	{
		for (p = proc_table; p < proc_table + NR_TASKS; p++)
		{
			if ((p->stat & WAIT) != 0 || p->dly > 0)
				continue; //若在等待状态或有睡眠时间，就不分配时间片
			else if (p->ticks > greatest_ticks)
			{
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks)
		{

			for (p = proc_table; p < proc_table + NR_TASKS; p++)
			{
				if ((p->stat & WAIT != 0) || p->dly > 0)
					continue; //若在等待状态或有睡眠时间，就不分配时间片
				p->ticks = p->req_ticks;
			}
		}
	}
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

PUBLIC int sys_dly(int k)
{
	p_proc_ready->dly = k;
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

PUBLIC void init_sem(int b)
{
	sem_head = 0;
	// semaphore1.available = 1; //设置理发椅子个数
	// semaphore2.available = b; //设置等待椅子个数
	// semaphore3.available = 0; //唤醒理发师，用于进程通信
	sp1 = sem_create(1);
	sp2 = sem_create(b);
	sp3 = sem_create(0);
}



PUBLIC void barber()
{
	print_str("I am sleeping\n", 1);
	while (1)
	{

		P(sp3);
		//理发

		print_str("customer ", 1);
		print_str(cname, 'a');
		print_str(" is having haircut\n", 1);

		dly(10000);
		V(sp1);

		print_str("haircut finished, customer ", 1);
		print_str(cname, 1);
		print_str(" is leaving\n", 1);

		// system_process_sleep(1000);
	}
}

PUBLIC void customer(char name, int color)
{
	char *out = "k";
	out[0] = name;
	P(sp2); //申请等待椅
			//得到等待椅子

	print_str("customer ", color);
	print_str(out, color);
	print_str(" comes in\n", color);

	P(sp1);
	//申请理发椅子，若成功，进行下面的语句

	V(sp2); //归还等待椅子

	//唤醒理发师
	cname[0] = name;
	V(sp3);
}