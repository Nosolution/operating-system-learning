
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
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
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY *p_tty);
PRIVATE u32 tty_do_read(TTY *p_tty);
PRIVATE void tty_do_write(TTY *p_tty);
PRIVATE void put_key(TTY *p_tty, u32 key);
PRIVATE void query(TTY *p_tty);
PRIVATE void color_keyword(TTY *p_tty, char color);
PRIVATE u32 find_last_char(TTY *p_tty);
PRIVATE void match(u32 *str, unsigned int size, u32 *key, unsigned int key_len, u32 *st_array, u32 *ed_array);
PRIVATE u32 *former_char(u32 *cur, u32 *head);
PRIVATE void append_char(u32 *p, u32 c, int size);
PRIVATE void cache_cursor(TTY *p_tty);
PRIVATE void reset(TTY *p_tty);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY *p_tty;

	init_keyboard();

	for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
	{
		init_tty(p_tty);
		reset(p_tty);
	}
	select_console(0);
	while (1)
	{
		for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
		{

			// timing
			if (get_ticks() - p_tty->st_ticks > p_tty->ticks_limit && !(p_tty->flag & FIND_MODE))
			{
				reset(p_tty);
			}
			else
			{
				u32 key = tty_do_read(p_tty);
				process(p_tty, key);
			}
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY *p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
	p_tty->cache_size = 0;
	p_tty->key_len = 0;
	p_tty->flag = 0;
	p_tty->st_ticks = get_ticks();
	p_tty->ticks_limit = 200000 * HZ / 1000;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void process(TTY *p_tty, u32 key)
{
	// char output[2] = {'\0', '\0'};
	if (key == NONE)
		return;
	if ((p_tty->flag & SHOW_MODE) && (key != ESC))
	{
		return;
	}

	else if ((p_tty->flag & SHOW_MODE) && (key == ESC))
	{
		backward_clean(p_tty->p_console, p_tty->key_len);
		color_keyword(p_tty, DEFAULT_CHAR_COLOR);
		p_tty->key_len = 0;
		p_tty->flag &= (!(FIND_MODE | SHOW_MODE));
	}
	else if (p_tty->flag & FIND_MODE) //in find_mode
	{

		if (!(key & FLAG_EXT))
		{
			out_colorful_char(p_tty->p_console, key, BLUE);
			append_char(p_tty->query_key, key, p_tty->key_len);
			p_tty->key_len++;
		}
		else
		{
			int raw_code = key & MASK_RAW;
			switch (raw_code)
			{
			case ESC:
				backward_clean(p_tty->p_console, p_tty->key_len);
				p_tty->key_len = 0;
				p_tty->flag &= (!FIND_MODE);
				break;
			case BACKSPACE:
				backward_clean(p_tty->p_console, 1);
				p_tty->key_len--;
				break;
			case ENTER:
				p_tty->flag |= SHOW_MODE;
				query(p_tty);
				color_keyword(p_tty, BLUE);
				break;
			default:
				break;
			}
		}
	}
	else //not in find_mode
	{
		if (!(key & FLAG_EXT))
		{
			//undo
			if (((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) && ((key & MASK_RAW) == 'z' || (key & MASK_RAW) == 'Z'))
			{
				if (p_tty->cache_size > 0)
				{
					p_tty->cache_size--;
					if (p_tty->char_cache[p_tty->cache_size] != '\b')
					{
						int steps = p_tty->cursor_cache[p_tty->cache_size] - p_tty->cursor_cache[p_tty->cache_size - 1];
						backward_clean(p_tty->p_console, steps);
					}
					else
					{
						u32 c = find_last_char(p_tty);
						out_char(p_tty->p_console, c);
					}
				}
			}
			//normal char
			else
			{
				out_char(p_tty->p_console, key);
				append_char(p_tty->char_cache, key, p_tty->cache_size);
				cache_cursor(p_tty);
				p_tty->cache_size++;

				// u8 s[1];
				// s[0] = p_tty->cursor_cache[p_tty->cache_size - 1] + '0';
				// tty_write(p_tty, s, 1);
				// p_tty->p_console->cursor--;
			}
		}
		else
		{
			int raw_code = key & MASK_RAW;
			switch (raw_code)
			{
			case ESC:
				p_tty->flag |= FIND_MODE;
				break;
			case BACKSPACE:
				out_char(p_tty->p_console, '\b');
				append_char(p_tty->char_cache, '\b', p_tty->cache_size);
				cache_cursor(p_tty);
				p_tty->cache_size++;
				break;
			case ENTER:
				out_char(p_tty->p_console, '\n');
				append_char(p_tty->char_cache, '\n', p_tty->cache_size);
				cache_cursor(p_tty);
				p_tty->cache_size++;
				break;
			case TAB:
				out_char(p_tty->p_console, '\t');
				append_char(p_tty->char_cache, '\t', p_tty->cache_size);
				cache_cursor(p_tty);
				p_tty->cache_size++;
			default:
				break;
			}
		}
	}
}
PRIVATE void query(TTY *p_tty)
{
	match(p_tty->char_cache, p_tty->cache_size, p_tty->query_key, p_tty->key_len, p_tty->st_array, p_tty->ed_array);
}

PRIVATE void color_keyword(TTY *p_tty, char color)
{
	unsigned int i = 0;
	while (i < MAX_CC_SIZE && p_tty->ed_array[i] != 0xFFFFFFFF)
	{
		// u8 s[3];
		// // s[0] = p_tty->cursor_cache[p_tty->st_array[i]] + '0';
		// s[0] = p_tty->st_array[i] + '0';
		// s[1] = ' ';
		// s[2] = p_tty->ed_array[i] + '0';
		// tty_write(p_tty, s, 3);
		unsigned int st = (p_tty->st_array[i] == HEAD ? 0 : p_tty->cursor_cache[p_tty->st_array[i]]);
		unsigned int ed = p_tty->cursor_cache[p_tty->ed_array[i]];
		set_color(p_tty->p_console, st, ed, color);
		i++;
	}
}

PRIVATE u32 find_last_char(TTY *p_tty)
{
	u32 *last_char = former_char(p_tty->char_cache + p_tty->cache_size, p_tty->char_cache);
	return last_char == NULL ? 0 : *last_char;
}

PRIVATE void match(u32 *str, unsigned int size, u32 *key, unsigned int key_len, u32 *st_array, u32 *ed_array)
{
	memset(st_array, 0xFF, 4 * MAX_CC_SIZE);
	memset(ed_array, 0xFF, 4 * MAX_CC_SIZE);
	u32 *cur_idx = former_char(str + size, str);
	int res_len = 0;

	while (cur_idx != NULL)
	{

		int i = key_len - 1;
		u32 *ed_idx = cur_idx;
		while (i >= 0 && (cur_idx != NULL))
		{
			if (*cur_idx != key[i])
				break;
			i--;
			cur_idx = former_char(cur_idx, str);
		}
		if (i < 0)
		{
			// cur_idx = (cur_idx == NULL ? HEAD : former_char(cur_idx, str)); //idx before printing
			// if (cur_idx == HEAD)
			// 	st_array[res_len] = HEAD;
			// else if (cur_idx == NULL)
			// 	st_array[res_len] = 0;
			// else
			// 	st_array[res_len] = cur_idx - str;

			st_array[res_len] = (cur_idx == NULL ? HEAD : (cur_idx - str));
			ed_array[res_len] = ed_idx - str;
			res_len++;

			// cur_idx = (cur_idx == NULL ? HEAD : cur_idx);
		}
		else
		{
			cur_idx = former_char(ed_idx, str); //retest from the former char of the outside loop
		}
	}
}

PRIVATE u32 *former_char(u32 *cur, u32 *head)
{
	u32 *p = cur - 1;
	int valid_ct = 0;
	while (p >= head && valid_ct <= 0)
	{
		valid_ct += (*p == '\b' ? -1 : 1);
		p--;
	}
	return valid_ct > 0 ? (p + 1) : NULL;
}

PRIVATE void cache_cursor(TTY *p_tty)
{
	p_tty->cursor_cache[p_tty->cache_size] = p_tty->p_console->cursor;
}

PRIVATE void append_char(u32 *p, u32 c, int size)
{
	p[size] = c;
}

PRIVATE void reset(TTY *p_tty)
{
	p_tty->cache_size = 0;
	p_tty->key_len = 0;
	p_tty->flag = 0;
	p_tty->st_ticks = get_ticks();

	backward_clean(p_tty->p_console, p_tty->p_console->cursor - p_tty->p_console->original_addr);
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY *p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES)
	{
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES)
		{
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}

/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE u32 tty_do_read(TTY *p_tty)
{
	if (is_current_console(p_tty->p_console))
	{
		u32 key = keyboard_read();
		return key;
	}
	return NONE;
}

/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY *p_tty)
{
	if (p_tty->inbuf_count)
	{
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
		{
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY *p_tty, char *buf, int len)
{
	char *p = buf;
	int i = len;

	while (i)
	{
		out_char(p_tty->p_console, *(p++));
		i--;
	}
}

/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char *buf, int len, PROCESS *p_proc)
{
	tty_write(&tty_table[p_proc->nr_tty], buf, len);
	return 0;
}
