
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				tty.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_TTY_H_
#define _ORANGES_TTY_H_


#define TTY_IN_BYTES	256	/* tty input queue size */
#define MAX_CC_SIZE 1024
#define MAX_KEY_LEN 256
#define FIND_MODE 1
#define SHOW_MODE 2
#define NONE 0

struct s_console;

/* TTY */
typedef struct s_tty
{
	u32	in_buf[TTY_IN_BYTES];	/* TTY 输入缓冲区 */
	u32*	p_inbuf_head;		/* 指向缓冲区中下一个空闲位置 */
	u32*	p_inbuf_tail;		/* 指向键盘任务应处理的键值 */
	int	inbuf_count;		/* 缓冲区中已经填充了多少 */

	u32 char_cache[MAX_CC_SIZE];
	u32 cursor_cache[MAX_CC_SIZE];
	int cache_size;
	u32 query_key[MAX_KEY_LEN];
	int key_len;
	unsigned int flag;

	u32 st_array[MAX_CC_SIZE];
	u32 ed_array[MAX_CC_SIZE];

	int st_ticks;
	int ticks_limit;

	struct s_console *	p_console;
}TTY;


#endif /* _ORANGES_TTY_H_ */
