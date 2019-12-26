
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
#include "stdio.h"

#define READING 0
#define WRITING 1

int read_cnt, write_cnt, cur_reader_id, cur_process; // processType = 0, 意为当前进程为读者；为1则为写者
SEMAPHORE rw_sem, writer_sem, mutex;
SEMAPHORE *rmutex, *wmutex, *x, *y, *z, *read_available;
/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
    disp_str("-----\"kernel_mainworld\" begins-----\n");

    TASK *p_task = task_table;
    PROCESS *p_proc = proc_table;
    char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16 selector_ldt = SELECTOR_LDT_FIRST;
    int i;
    for (i = 0; i < NR_TASKS; i++)
    {
        strcpy(p_proc->p_name, p_task->name); // name of the process
        p_proc->pid = i;                      // pid

        p_proc->ldt_sel = selector_ldt;

        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
               sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
               sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
        p_proc->regs.cs = ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.ds = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.es = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.fs = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.ss = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

        p_proc->regs.eip = (u32)p_task->initial_eip;
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1 << 3;
    }

    read_cnt = 0;
    cur_reader_id = 0;
    cur_process = WRITING;

    for (int i = 0; i < NR_TASKS; i++)
    {
        proc_table[i].ticks = proc_table[i].req_ticks = 1;
    }

    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;

    init_clock();
    init_keyboard();

    restart();

    while (1)
    {
    }
}

void observer()
{
    while (1)
    {
        if (cur_process == READING)
        {
            print_str("Currently is reading\n", DEFAULT_CHAR_COLOR);
            printn(read_cnt, DEFAULT_CHAR_COLOR);
            print_str(" readers are reading.\n", DEFAULT_CHAR_COLOR);
        }
        else
        {
            print_str("Currently is writing\n", DEFAULT_CHAR_COLOR);
        }
        milli_delay(TIMESLICE);
    }
}

/*======================================================================*
                            readers and writers
 *======================================================================*/
void reader(int time)
{
    while (1)
    {
        if (PRIORITY == WRITING)
        {
            P(&writer_sem);
        }
        P(&mutex);
        if (read_cnt == 0)
        {
            P(&rw_sem);
        }
        cur_reader_id++;
        int local_reader_id = cur_reader_id;
        if (read_cnt >= POSITIONS)
        {
            V(&mutex);
            if (PRIORITY == WRITING)
            {
                V(&writer_sem);
            }
            print_str("The reader NO.", DEFAULT_CHAR_COLOR);
            printn(local_reader_id, DEFAULT_CHAR_COLOR);
            print_str(" comes and leaves for no positions.\n");
            dly(time * TIMESLICE);
        }
        else
        {
            read_cnt++;
            V(&mutex);
            if (PRIORITY == WRITING)
            {
                V(&writer_sem);
            }
            print_str("The reader NO.", DEFAULT_CHAR_COLOR);
            printn(local_reader_id, DEFAULT_CHAR_COLOR);
            print_str(" comes and starts reading.\n", DEFAULT_CHAR_COLOR);
            cur_process = READING;

            dly(time * TIMESLICE);

            print_str("The reader NO.", DEFAULT_CHAR_COLOR);
            printn(local_reader_id, DEFAULT_CHAR_COLOR);
            print_str(" has finished his book and left.\n", DEFAULT_CHAR_COLOR);
            P(&mutex);
            read_cnt--;
            if (read_cnt == 0)
            {
                V(&rw_sem);
            }
            V(&mutex);
        }
    }
}

void reader_A()
{
    reader(2);
}

void reader_B()
{
    reader(3);
}

void reader_C()
{
    reader(3);
}

void writer(int time)
{
    while (1)
    {
        if (PRIORITY == WRITING)
        {
            P(&writer_sem);
        }
        P(&rw_sem);
        print_str("The writer starts writing!\n", DEFAULT_CHAR_COLOR);
        cur_process = WRITING;

        dly(time * TIMESLICE);

        print_str("The writer finished writing!\n", DEFAULT_CHAR_COLOR);
        V(&rw_sem);
        if (PRIORITY == WRITING)
        {
            V(&writer_sem);
        }
    }
}

void writer_D()
{
    writer(3);
}

void writer_E()
{
    writer(4);
}

void preader(int time)
{
    while (1)
    {
        P(read_available);
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
        v(rmutex);
#endif

        print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
        print_str(" started reading\n", DEFAULT_CHAR_COLOR);

        dly(time * TIMESLICE);

        print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
        print_str(" finished reading\n", DEFAULT_CHAR_COLOR);

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
        v(rmutex);
#endif
        V(read_available);
    }
}

void pwriter(int time)
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

        P(wmutex);
        print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
        print_str(" started reading\n", DEFAULT_CHAR_COLOR);

        dly(time * TIMESLICE);

        print_str(p_proc_ready->p_name, DEFAULT_CHAR_COLOR);
        print_str(" finished reading\n", DEFAULT_CHAR_COLOR);
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