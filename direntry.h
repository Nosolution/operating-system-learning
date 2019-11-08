#include "commons.h"

struct DirEntry
{
    byte dirname[11];
    byte dirattr;
    byte reserve[10];
    number wrttime;
    number wrtdate;
    number fstclus;
    number filesize;
};
const int deatrofs[] = {0, 11, 12, 22, 24, 26, 28, 32};
DirEntry parse_entry(byte *entry);