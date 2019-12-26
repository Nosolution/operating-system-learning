
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				tty.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_TTY_H_
#define _ORANGES_TTY_H_


#define TTY_IN_BYTES	256	/* tty input queue size */
#define MAX_HISTORY_SIZE 1024	//设置的历史记录的最长大小，因为作业要求中每隔20秒刷新，普通人输入必不会到达上限，因此在代码中没有实现溢出检查逻辑
#define MAX_KEY_LEN 256			//关键字的最长大小，情况同上
#define FIND_MODE 1
#define SHOW_MODE 2
#define NULL 0
#define HEAD -1 

struct s_console;

/* TTY */
typedef struct s_tty
{
	u32	in_buf[TTY_IN_BYTES];	/* TTY 输入缓冲区 */
	u32*	p_inbuf_head;		/* 指向缓冲区中下一个空闲位置 */
	u32*	p_inbuf_tail;		/* 指向键盘任务应处理的键值 */
	int	inbuf_count;		/* 缓冲区中已经填充了多少 */

	u32 char_history[MAX_HISTORY_SIZE];		//tty中已输入的字符(包括可见和不可见)的记录
	u32 cursor_history[MAX_HISTORY_SIZE];	//每次输入字符后的光标的位置的记录
	int history_size;						//历史记录的长度
	u32 query_key[MAX_KEY_LEN];				//查询关键词
	int key_len;							//关键词长度
	u32 normal_history_size;				//normal_mode的历史记录长度，进入find_mode时备份
	unsigned int flag;						//一些关于此tty的标志位

	u32 st_array[MAX_HISTORY_SIZE];			//被匹配单词的开始位置的数组
	u32 ed_array[MAX_HISTORY_SIZE];			//被匹配单词的结束位置的数组

	int st_ticks;							//任务开始时的tick数
	int ticks_limit;						//限定的任务持续时间(即刷新间隔)

	struct s_console *	p_console;
}TTY;


#endif /* _ORANGES_TTY_H_ */
