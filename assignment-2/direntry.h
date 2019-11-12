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
//abstolute offset of each attr in bytes
const int deatrofs[] = {0, 11, 12, 22, 24, 26, 28, 32};
//count of attrs
const int de_attr_ct = 7;
//entry attrs
const byte READ_ONLY = 0x01; 
const byte HIDDEN = 0x02;
const byte SYSTEM = 0x04;
const byte VOLUME_ID = 0x08;
const byte DIRECTORY = 0x10;
const byte ARCHIVE = 0x20;
DirEntry *parse_entry(byte *entry);