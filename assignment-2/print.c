extern void prints(const char *s, int color);  //print str in color
extern void printi(const int i);               //print number
extern void printcs(const char *s, int count); //print str in size
extern void asm_prints(const char *s, int color);  //print str in color
extern void asm_printi(const int i);               //print number
extern void asm_printcs(const char *s, int count); //print str in size

void prints(const char *s, int color)
{
    asm_prints(s, color);
}
extern void printi(const int i)
{
    asm_printi(i);
}
extern void printcs(const char *s, int count)
{
    asm_printcs(s, count);
}