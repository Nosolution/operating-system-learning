
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

PUBLIC READY_BLOCK *rb_head;
PUBLIC READY_BLOCK *rb_tail;
PUBLIC READY_BLOCK ready_queue[MAX_RB];
PUBLIC READY_BLOCK empty_block;
PUBLIC PROCESS proc_head;
PUBLIC PROCESS proc_tail;
PUBLIC SEMAPHORE sems[MAX_SEM];
int sem_head;
int slice_ticks = TIMESLICE + 10;
PUBLIC READY_BLOCK *alloc_rb();
PUBLIC void free_rb(READY_BLOCK *p);
int min(int a, int b);
PUBLIC void wait_cur();
PUBLIC int queue_not_full(int head, int tail, int capacity);
PUBLIC int mem_cmp(void *s1, void *s2, int n);

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	READY_BLOCK *next = rb_head->next;

	if (next != rb_tail)
	{
		print_str("\n", DEFAULT_CHAR_COLOR);
		for (READY_BLOCK *cur = rb_head; cur != rb_tail; cur = cur->next)
		{
			print_str(cur->p->p_name, DEFAULT_CHAR_COLOR);
		}
		print_str("\n", DEFAULT_CHAR_COLOR);
		PROCESS *p_cur, *p_next;
		p_cur = p_proc_ready;
		p_next = next->p;
		p_proc_ready = p_next;
		// print_str(p_proc_ready->p_name, BLUE);
		// print_str(" switch, ticks: ", BLUE);
		// printn(p_proc_ready->ticks, BLUE);
		// print_str("\n", DEFAULT_CHAR_COLOR);
		// p_next->ticks = min(p_next->req_ticks, TIMESLICE);
		rb_head->next = next->next;
		ready(p_cur);
		free_rb(next);
	}
}

PUBLIC void proc_tick()
{
	// print_str(p_proc_ready->p_name, BLUE);
	// print_str(" ticks: ", BLUE);
	// printn(p_proc_ready->ticks, BLUE);
	// print_str("\n", DEFAULT_CHAR_COLOR);

	PROCESS *p;
	for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
	{
		if (p->dly > 0)
		{
			p->dly--;
			if (p->dly == 0)
				ready(p);
		}
	}
	if (p_proc_ready->ticks > 0)
		p_proc_ready->ticks--;
	if (slice_ticks > 0)
		slice_ticks--;
	if (p_proc_ready->ticks == 0)
	{
		print_str(p_proc_ready->p_name, BLUE);
		print_str("path1pre\n", DEFAULT_CHAR_COLOR);
		if ((p_proc_ready->stat & MRUN) != 0)
		{
			print_str("path1mid\n", DEFAULT_CHAR_COLOR);
			p_proc_ready->stat &= ~MRUN;
		}
		print_str("path1post\n", DEFAULT_CHAR_COLOR);
		p_proc_ready->ticks = p_proc_ready->req_ticks;
		schedule();
	}
	else if (slice_ticks == 0)
	{
		print_str("path2\n", DEFAULT_CHAR_COLOR);
		slice_ticks = TIMESLICE + 100;
		schedule();
	}
}

PUBLIC void init_memory()
{
	memset(ready_queue, 0, MAX_RB * sizeof(READY_BLOCK));
	memset(sems, 0, MAX_SEM * sizeof(SEMAPHORE));
	memset(&empty_block, 0, sizeof(READY_BLOCK));
	sem_head = 0;

	rb_head = ready_queue;
	rb_head->p = &proc_head;
	rb_head->p->priority = 0;

	rb_tail = ready_queue + 1;
	rb_tail->p = &proc_tail;
	rb_tail->p->priority = 9;
	rb_head->next = rb_tail;
}

PUBLIC void ready(PROCESS *p)
{

	READY_BLOCK *prev = rb_head;
	READY_BLOCK *cur = rb_head->next;
	while (cur->p->priority <= p->priority)
	{
		prev = cur;
		cur = cur->next;
	}
	READY_BLOCK *rb = alloc_rb();
	rb->p = p;
	rb->next = cur;
	prev->next = rb;
}

PUBLIC void push_back(PROCESS *p)
{
	READY_BLOCK *last = rb_head;
	while (last->next != rb_tail)
		last = last->next;
	last->next = alloc_rb();
	last->next->p = p;
	last->next->next = rb_tail;
}

PUBLIC READY_BLOCK *alloc_rb()
{
	for (int i = 0; i < MAX_RB; i++)
	{
		if (mem_cmp(ready_queue + i, &empty_block, sizeof(READY_BLOCK)) == 0)
			return ready_queue + i;
	}
}

PUBLIC void free_rb(READY_BLOCK *p)
{
	memset(p, 0, sizeof(READY_BLOCK));
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

PUBLIC int sys_print_str(char *str, int color)
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

PUBLIC int printn(int n, int color)
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

	sys_print_str(num, color);
}

PUBLIC int sys_dly(int k)
{
	//TODO
	p_proc_ready->dly = k;
	schedule();
	return 0;
}

PUBLIC int sys_P(SEMAPHORE *t)
{

	t->available--;
	if (t->available < 0)
	{
		wait_cur();
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
		PROCESS *p = t->wait[t->wait_head];
		p->stat &= ~WAIT;
		ready(p);
		t->wait_head = (t->wait_head + 1) % MAX_WAITING;
	}
	return 0;
}

PUBLIC SEMAPHORE *sem_create(int cnt)
{
	SEMAPHORE *sem = sems + sem_head++;
	sem->available = cnt;
	sem->wait_head = 0;
	sem->wait_tail = 1;
	return sem;
}

PUBLIC void mock_run()
{
	// for (int i = 0; i < 50000; i++)
	// {
	// }

	// while (p_proc_ready->ticks <= p_proc_ready->req_ticks - 5)
	// {
	// }
	print_str(p_proc_ready->p_name, BLUE);
	p_proc_ready->stat |= MRUN;
	print_str("run\n", DEFAULT_CHAR_COLOR);
	while ((p_proc_ready->stat & MRUN) != 0)
	{
		/* code */
	}
	print_str(p_proc_ready->p_name, BLUE);
	print_str("finish1\n", DEFAULT_CHAR_COLOR);
}

int min(int a, int b)
{
	return a <= b ? a : b;
}

PUBLIC int mem_cmp(void *s1, void *s2, int n)
{
	char *p1 = (char *)s1;
	char *p2 = (char *)s2;
	for (int i = 0; i < n; i++)
	{
		if (p1[i] < p2[i])
			return -1;
		else if (p1[i] > p2[i])
			return 1;
	}
	return 0;
}
