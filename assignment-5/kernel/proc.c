
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
SEMAPHORE semaphore1;
SEMAPHORE semaphore2;
SEMAPHORE semaphore3;
SEMAPHORE *sp1;
SEMAPHORE *sp2;
SEMAPHORE *sp3;
char *cname = "1";

PUBLIC void schedule()
{
	PROCESS *p;
	int greatest_ticks = 0;

	for (p = proc_table; p < proc_table + NR_TASKS; p++)
	{
		if (p->sleep > 0)
		{
			p->sleep--; //减掉睡眠的时间
		}
	}
	while (!greatest_ticks)
	{
		for (p = proc_table; p < proc_table + NR_TASKS; p++)
		{
			if (p->wait > 0 || p->sleep > 0)
			{
				continue; //若在等待状态或有睡眠时间，就不分配时间片
			}
			if (p->ticks > greatest_ticks)
			{
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks)
		{

			for (p = proc_table; p < proc_table + NR_TASKS; p++)
			{
				if (p->wait > 0 || p->sleep > 0)
				{
					continue; //若在等待状态或有睡眠时间，就不分配时间片
				}
				p->ticks = p->priority;
			}
		}
	}
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
		out_char_color(p_tty->p_console, str[i], color);
		i++;
	}
}

PUBLIC int sys_dly(int k)
{
	p_proc_ready->sleep = k;
	schedule();
	return 0;
}

PUBLIC int sys_P(SEMAPHORE *t)
{

	t->s--;
	if (t->s < 0)
	{
		p_proc_ready->wait = 1;
		t->x[t->ptr] = p_proc_ready;
		t->ptr++; //进入等待进程队列

		schedule();
	}
}

PUBLIC int sys_V(SEMAPHORE *t)
{
	t->s++;
	if (t->s <= 0)
	{
		t->x[0]->wait = 0;
		for (int i = 0; i < t->ptr; i++)
		{
			t->x[i] = t->x[i + 1];
		}
		t->ptr--;
	}
}

PUBLIC void initSemaphore(int b)
{
	semaphore1.s = 1; //设置理发椅子个数
	semaphore2.s = b; //设置等待椅子个数
	semaphore3.s = 0; //唤醒理发师，用于进程通信
	sp1 = &semaphore1;
	sp2 = &semaphore2;
	sp3 = &semaphore3;
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