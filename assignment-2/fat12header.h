#include "commons.h"

struct Fat12Header
{
    byte BS_OEMName[8];     //OEM字符串，必须为8个字符，不足以空格填空
    number BPB_BytsPerSec;  //每扇区字节数, 默认为512
    number BPB_SecPerClus;  //每簇占用的扇区数, 默认为1
    number BPB_RsvdSecCnt;  //Boot占用的扇区数, 默认为1
    number BPB_NumFATs;     //FAT表的记录数, 默认为2
    number BPB_DirEntCnt;   //最大根目录文件数
    number BPB_TotSec16;    //逻辑扇区总数，如果值是0，说明磁盘中有超过2^16-1个扇区，并且真实值保存在 Large Sector Count字段，在0x20处
    byte BPB_Media;         //媒体描述符
    number BPB_FATSz16;     //每个FAT占用扇区数
    number BPB_SecPerTrk;   //每个磁道扇区数
    number BPB_NumHeads;    //磁头数
    number BPB_HiddSec;     //隐藏扇区数
    number BPB_TotSec32;    //如果BPB_TotSec16是0，则在这里记录
    byte BS_DrvNum;         //中断13的驱动器号
    byte BS_Reserved1;      //未使用
    byte BS_BootSig;        //扩展引导标志
    number BS_VolID;        //卷序列号
    byte BS_VolLab[11];     //卷标，必须是11个字符，不足以空格填充
    byte BS_FileSysType[8]; //文件系统类型，必须是8个字符，不足填充空格
};
//abstolute offset of each attr in bytes
const int hatrofs[] = {0, 3, 11, 13, 14, 16, 17, 19, 21, 22, 24, 26, 28, 32, 36, 37, 38, 39, 43, 54, 62, 510, 512};
//count of attrs
const int hd_attr_ct = 22;
Fat12Header* parse_header(const byte *);