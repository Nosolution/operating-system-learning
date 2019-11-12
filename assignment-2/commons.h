typedef unsigned short fatnum;
typedef unsigned char byte;
typedef unsigned int number;

#define _BLOCK_SIZE 512
#define _FAT_ITEM_LEN 12
#define _DIR_ITEM_SIZE 32
// #define _BPB_ST 0
// #define _FAT_ST 1
// #define _ROOT_ENTRY_ST 19
// #define _DATA_ST 33
#define fat_bias(x) x - 2
#define _FAT_END 0xFF7
#define _CUR_DIRNAME "." 
#define _PARENT_DIRNAME ".." 

number bs2n(const byte *bytes, int n);