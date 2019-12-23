
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
PRIVATE u32 *former_char_ptr(u32 *cur, u32 *head);
PRIVATE void append_char(u32 *p, u32 c, int size);
PRIVATE void record_cursor(TTY *p_tty);
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
	p_tty->history_size = 0;
	p_tty->normal_history_size = 0;
	p_tty->key_len = 0;
	p_tty->flag = 0;
	p_tty->st_ticks = get_ticks();
	p_tty->ticks_limit = 2000000 * HZ / 1000;

	init_screen(p_tty);
}

/*======================================================================*
				process
 *======================================================================*/
PUBLIC void process(TTY *p_tty, u32 key)
{
	if (key == NONE)
		return;
	if ((p_tty->flag & SHOW_MODE) && (key != ESC))
	{
		//do nothing
		return;
	}
	//exit show_mode
	else if ((p_tty->flag & SHOW_MODE) && (key == ESC))
	{
		backward_clean(p_tty->p_console,
					   p_tty->cursor_history[p_tty->history_size - 1] - p_tty->cursor_history[p_tty->normal_history_size - 1]);
		color_keyword(p_tty, DEFAULT_CHAR_COLOR);
		p_tty->history_size = p_tty->normal_history_size;
		p_tty->key_len = 0;
		p_tty->flag &= (!(FIND_MODE | SHOW_MODE));
	}
	else if (p_tty->flag & FIND_MODE) //in find_mode
	{
		if (!(key & FLAG_EXT))
		{
			//undo
			if (((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) && ((key & MASK_RAW) == 'z' || (key & MASK_RAW) == 'Z'))
			{
				if (p_tty->history_size > 0) //can undo
				{
					if (p_tty->char_history[p_tty->history_size - 1] != '\b') //need to clean a char
					{
						//in find_mode cannot clean white chars
						if (p_tty->key_len > 0)
						{
							p_tty->history_size--;
							int steps = p_tty->cursor_history[p_tty->history_size] - p_tty->cursor_history[p_tty->history_size - 1];
							backward_clean(p_tty->p_console, steps);
							p_tty->key_len--;
						}
					}
					else //need to rewrite a char
					{
						p_tty->history_size--;
						u32 c = find_last_char(p_tty);
						out_colorful_char(p_tty->p_console, c, BLUE);
						append_char(p_tty->query_key, c, p_tty->key_len);
						p_tty->key_len++;
					}
				}
			}
			//normal char
			else
			{
				out_colorful_char(p_tty->p_console, key, BLUE);
				append_char(p_tty->char_history, key, p_tty->history_size);
				record_cursor(p_tty);
				p_tty->history_size++;

				append_char(p_tty->query_key, key, p_tty->key_len);
				p_tty->key_len++;
			}
		}
		else
		{
			int raw_code = key & MASK_RAW;
			u32 *p;
			switch (raw_code)
			{
			case ESC: //exit find_mode
				backward_clean(p_tty->p_console,
							   p_tty->cursor_history[p_tty->history_size - 1] - p_tty->cursor_history[p_tty->normal_history_size - 1]);
				p_tty->history_size = p_tty->normal_history_size; //restore history size of normal mode
				p_tty->key_len = 0;
				p_tty->flag &= (!FIND_MODE);
				break;
			case BACKSPACE:
				// out_char(p_tty->p_console, '\b');
				p = former_char_ptr(p_tty->char_history + p_tty->history_size, p_tty->char_history);
				if (p - p_tty->char_history >= p_tty->normal_history_size)
				{
					if (*p == '\t')
					{
						int steps = p_tty->cursor_history[p_tty->history_size - 1] - (p == p_tty->char_history ? 0 : p_tty->cursor_history[p - p_tty->char_history - 1]);
						backward_clean(p_tty->p_console, steps);
					}
					else
					{
						out_char(p_tty->p_console, '\b');
					}
					append_char(p_tty->char_history, '\b', p_tty->history_size);
					record_cursor(p_tty);
					p_tty->history_size++;
					p_tty->key_len--;
				}
				break;
			case TAB:
				out_char(p_tty->p_console, '\t');
				append_char(p_tty->char_history, '\t', p_tty->history_size);
				record_cursor(p_tty);
				p_tty->history_size++;
				append_char(p_tty->query_key, '\t', p_tty->key_len);
				p_tty->key_len++;
				break;
			case ENTER: //enter show_mode
				p_tty->flag |= SHOW_MODE;
				query(p_tty);
				color_keyword(p_tty, BLUE);
				break;
			default:
				break;
			}
		}
	}
	else //in normal_mode
	{
		if (!(key & FLAG_EXT))
		{
			//undo
			if (((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) && ((key & MASK_RAW) == 'z' || (key & MASK_RAW) == 'Z'))
			{
				if (p_tty->history_size > 0)
				{
					p_tty->history_size--;
					if (p_tty->char_history[p_tty->history_size] != '\b')
					{
						int steps = p_tty->cursor_history[p_tty->history_size] - p_tty->cursor_history[p_tty->history_size - 1];
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
				append_char(p_tty->char_history, key, p_tty->history_size);
				record_cursor(p_tty);
				p_tty->history_size++;
			}
		}
		else
		{
			int raw_code = key & MASK_RAW;
			u32 *p;
			switch (raw_code)
			{
			case ESC:
				p_tty->flag |= FIND_MODE;
				p_tty->normal_history_size = p_tty->history_size; //store history size in normal mode
				break;
			case BACKSPACE:
				// out_char(p_tty->p_console, '\b');
				p = former_char_ptr(p_tty->char_history + p_tty->history_size, p_tty->char_history);
				if (p != NULL)
				{
					if ((*p == '\t') || (*p == '\n'))
					{
						int steps = p_tty->cursor_history[p_tty->history_size - 1] - (p == p_tty->char_history ? 0 : p_tty->cursor_history[p - p_tty->char_history - 1]);
						backward_clean(p_tty->p_console, steps);
					}
					else
					{
						out_char(p_tty->p_console, '\b');
					}
					append_char(p_tty->char_history, '\b', p_tty->history_size);
					record_cursor(p_tty);
					p_tty->history_size++;
				}
				break;
			case TAB:
				out_char(p_tty->p_console, '\t');
				append_char(p_tty->char_history, '\t', p_tty->history_size);
				record_cursor(p_tty);
				p_tty->history_size++;
				break;
			case ENTER:
				out_char(p_tty->p_console, '\n');
				append_char(p_tty->char_history, '\n', p_tty->history_size);
				record_cursor(p_tty);
				p_tty->history_size++;
				break;
			default:
				break;
			}
		}
	}
}
/**
 * Query string on the screen with stored keyword
 * @param	p_tty	specified tty 
 */
PRIVATE void query(TTY *p_tty)
{
	match(p_tty->char_history, p_tty->normal_history_size, p_tty->query_key, p_tty->key_len, p_tty->st_array, p_tty->ed_array);
}

/**
 * Color given word on the screen. Target words need to be matched prior
 * @param	p_tty	specified tty
 * @param	color	chosen color
 */
PRIVATE void color_keyword(TTY *p_tty, char color)
{
	unsigned int i = 0;
	while (i < MAX_HISTORY_SIZE && p_tty->ed_array[i] != 0xFFFFFFFF)
	{
		unsigned int st = (p_tty->st_array[i] == HEAD ? 0 : p_tty->cursor_history[p_tty->st_array[i]]);
		unsigned int ed = p_tty->cursor_history[p_tty->ed_array[i]];
		set_color(p_tty->p_console, st, ed, color);
		i++;
	}
}

/**
 * Find the last visible char on screen of the given tty
 * @param	p_tty	given tty
 */
PRIVATE u32 find_last_char(TTY *p_tty)
{
	u32 *last_char = former_char_ptr(p_tty->char_history + p_tty->history_size, p_tty->char_history);
	return last_char == NULL ? 0 : *last_char;
}

/**
 * Match keyword with given string
 * @param 	str			ptr to the the string
 * @param	size		size of the string
 * @param	key			ptr to the keyword
 * @param	key_len		length of the keyword
 * @param	st_array	storage location of the starting position of the words matched
 * @param	ed_array	storage location of the ending position of the words matched
 */
PRIVATE void match(u32 *str, unsigned int size, u32 *key, unsigned int key_len, u32 *st_array, u32 *ed_array)
{
	memset(st_array, 0xFF, 4 * MAX_HISTORY_SIZE);
	memset(ed_array, 0xFF, 4 * MAX_HISTORY_SIZE);
	u32 *cur_idx = former_char_ptr(str + size, str);
	int res_len = 0;

	while (cur_idx != NULL)
	{

		int i = key_len - 1;
		//ed_idx is the starting point of check
		u32 *ed_idx = cur_idx;
		while (i >= 0 && (cur_idx != NULL))
		{
			if (*cur_idx != key[i])
				break;
			i--;
			cur_idx = former_char_ptr(cur_idx, str);
		}
		if (i < 0) //keyword is found
		{
			//judge if has reached the head of str
			st_array[res_len] = (cur_idx == NULL ? HEAD : (cur_idx - str));
			ed_array[res_len] = ed_idx - str;
			res_len++;
		}
		else
		{
			//next check starts from the former char of current starting point
			cur_idx = former_char_ptr(ed_idx, str);
		}
	}
}

/**
 * Find the idx of former visible char(meaning not covered by backplace) of current position in an array.
 * @param	cur		current position, the pos of found char will be preceding
 * @param	head	the head of given array, to identify the range 
 */
PRIVATE u32 *former_char_ptr(u32 *cur, u32 *head)
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

/**
 * Record current position of the cursor of given tty.
 * @param	p_tty	given tty
 */
PRIVATE void record_cursor(TTY *p_tty)
{
	p_tty->cursor_history[p_tty->history_size] = p_tty->p_console->cursor;
}

/**
 * Append the char to the end of the given array.
 * @param	p		ptr to the head of specified array
 * @param	c		given char
 * @param	size	current size of the specified array
 */
PRIVATE void append_char(u32 *p, u32 c, int size)
{
	p[size] = c;
}

/**
 * Reset the tty feilds and screen
 * @param	p_tty 	specified tty
 */
PRIVATE void reset(TTY *p_tty)
{
	p_tty->history_size = 0;
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
