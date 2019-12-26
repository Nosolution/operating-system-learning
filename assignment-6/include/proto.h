
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void A();
void B();
void C();
void D();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();
PUBLIC u32 keyboard_read();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC void out_colorful_char(CONSOLE* p_con, char ch,char color);
PUBLIC void set_color(CONSOLE *p_con, unsigned int st_cursor, unsigned int ed_cursor, char color);
PUBLIC void init_screen(TTY *p_tty);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);
PUBLIC void backward_clean(CONSOLE *p_con, unsigned int count);

/* printf.c */
PUBLIC  int     printf(const char *fmt, ...);

/* vsprintf.c */
PUBLIC  int     vsprintf(char *buf, const char *fmt, va_list args);

/* 以下是系统调用相关 */

/* 系统调用 - 系统级 */
/* proc.c */
PUBLIC  int     sys_get_ticks();
PUBLIC  int     sys_write(char* buf, int len, PROCESS* p_proc);
PUBLIC int  sys_P(SEMAPHORE* t);
PUBLIC int  sys_V(SEMAPHORE* t);
PUBLIC int  sys_print_str(char * str, int color);
PUBLIC int  sys_dly(int k);
PUBLIC void init_sem(int b);
PUBLIC void barber();
PUBLIC void customer(char name,int color);
/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */

/* 系统调用 - 用户级 */
PUBLIC  int     get_ticks();
PUBLIC  void    write(char* buf, int len);

