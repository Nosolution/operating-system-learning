/**
 * This file is intended to act as a proxy of nasm program for cpp program. 
 * Because direct connection of the two kinds of programs derives mysterious bugs about memory, 
 * which is beyond my ability to resolve with a time limitation.
 */

extern void prints(const char *s, int color);  
extern void printi(const int i);               
extern void printcs(const char *s, int count); 
extern void asm_prints(const char *s, int color);  //print str in color
extern void asm_printi(const int i);               //print number
extern void asm_printcs(const char *s, int count); //print str in size

/**
 * Print a string/char sequence until '\0' are met.
 * @param   s   start of the string
 * @param   color   the color str will be printed in.
 */
void prints(const char *s, int color)
{
    asm_prints(s, color);
}

/**
 * Print an integer number.
 * @param   i   the number will be print
 */
void printi(const int i)
{
    asm_printi(i);
}

/**
 * Print chars in specific number.
 * @param   the start ptr of the chars
 * @param   count number of chars needed to be printed
 */
void printcs(const char *s, int count)
{
    asm_printcs(s, count);
}