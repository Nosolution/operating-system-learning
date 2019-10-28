#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include "dirnode.h"
#define _BLOCK_SIZE 512
#define _FAT_ITEM_LEN 12
#define _DIR_ITEM_SIZE 32
#define _BPB_ST 0
#define _FAT_ST 1
#define _ROOT_ENTRY_ST 19
#define _DATA_ST 33
#define fat_bias(x) x - 2
#define _PARSE_SUCCESS 0
#define _WRONG_COMMAND 1
#define _WRONG_OPTION 2
#define _TOO_MUCH_PARAM 3
#define _NO_PARAM 4
#define _WHITE_COLOR 10
#define _RED_COLOR 12
typedef unsigned short fatnum;
typedef unsigned char byte;
typedef unsigned int number;

struct Fat12Header
{
    byte BS_OEMName[8];     //OEM字符串，必须为8个字符，不足以空格填空
    number BPB_BytsPerSec;  //每扇区字节数, 默认为512
    number BPB_SecPerClus;  //每簇占用的扇区数, 默认为1
    number BPB_RsvdSecCnt;  //Boot占用的扇区数, 默认为1
    number BPB_NumFATs;     //FAT表的记录数
    number BPB_DirEntCnt;   //最大根目录文件数
    number BPB_TotSec16;    //每个FAT占用扇区数
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

const int hatrofs[] = {0, 3, 11, 13, 14, 16, 17, 19, 21, 22, 24, 26, 28, 32, 36, 37, 38, 39, 43, 54, 62, 510, 512};
const int deatrofs[] = {0, 11, 12, 22, 24, 26, 28, 32};
const int hd_attr_ct = 22;
const int de_attr_ct = 7;
const byte READ_ONLY = 0x01;
const byte HIDDEN = 0x02;
const byte SYSTEM = 0x04;
const byte VOLUME_ID = 0x08;
const byte DIRECTORY = 0x10;
const byte ARCHIVE = 0x20;

using namespace std;
Fat12Header &parse_header(char *);
DirEntry &parse_entry(char *entry);
number bs2n(char *bytes, int n);
fatnum *get_fatlist(char *bytes);
DirNode &read_meta_data(DirNode &node, const fatnum *fatlist, ifstream &fs);
byte *read_data(const number fstclus, const number size, const fatnum *fatlist, ifstream &fs);
int parse_cmd(string &line, string &cmd, string &opt, string &param);
const char *next_word(const char *st, int &len, string &word);
void bfs_print_node(DirNode *node, void (*print)(DirNode *node));
void plain_print(DirNode *node);
void detailed_print(DirNode *node);
void asm_prints(const char *s, int color);
void asm_printi(const int i);
void hint();
int main()
{
    ifstream img;
    img.open("img/data.img", ios::binary);
    char header_bytes[_BLOCK_SIZE];
    img.seekg(_BLOCK_SIZE * _BPB_ST, ios::beg);
    img.read(header_bytes, _BLOCK_SIZE);
    Fat12Header header = parse_header(header_bytes);

    char fat_bytes[_BLOCK_SIZE * 9];
    img.seekg(_BLOCK_SIZE * _FAT_ST, ios::beg);
    img.read(fat_bytes, _BLOCK_SIZE * 9);
    fatnum *fatlist = get_fatlist(fat_bytes);

    DirNode root{"/", true};
    img.seekg(_BLOCK_SIZE * _ROOT_ENTRY_ST, ios::beg);
    for (int i = 0; i < header.BPB_DirEntCnt; i++)
    {
        read_meta_data(root, fatlist, img);
        img.seekg(_DIR_ITEM_SIZE, ios::cur);
    }
    cout << "Hello World!" << endl;
    string line, cmd, opt, param;
    int code;
    while (true)
    {
        getline(cin, line);
        code = parse_cmd(line, cmd, opt, param);
        if (code == _PARSE_SUCCESS)
        {
            if (cmd.compare("quit") == 0)
                break;
            else if (cmd.compare("ls") == 0)
            {
                char main_dir[60] = "main";
                strcat(main_dir, param.c_str);
                root.filename = "main"; //make method perform consistantly
                DirNode *cur = root.find(main_dir);
                root.filename = "/";

                if (opt.length() > 0)
                {
                    bfs_print_node()
                }
                else
                {
                    what
                }
            }
        }
        else
        {
            hint();
        }
    }
    return 0;
}

DirNode &read_meta_data(DirNode &node, const fatnum *fatlist, ifstream &fs)
{
    if (node.is_dir)
    {
        DirEntry *entry = (DirEntry *)node.entry;
        byte *data = read_data(entry->fstclus, entry->filesize, fatlist, fs);
        byte *entry_bytes = new byte[_DIR_ITEM_SIZE];
        int count = 0;
        DirEntry chd_entry;

        while (count * _DIR_ITEM_SIZE < (*entry).filesize)
        {
            memcpy(entry_bytes, data + count * _DIR_ITEM_SIZE, _DIR_ITEM_SIZE);
            chd_entry = parse_entry(entry_bytes);
            DirNode child{(char *)chd_entry.dirname, chd_entry.dirattr ^ DIRECTORY == 0};
            child.entry = &chd_entry;

            read_meta_data(child, fatlist, fs); //dfs

            node.add_child(&child);
            count++;
        }
    }
    return node;
}

byte *read_data(const number fstclus, const number size, const fatnum *fatlist, ifstream &fs)
{
    byte *data = new byte[size];
    char blc_buf[_BLOCK_SIZE];
    int count = 0;
    for (number i = fstclus; i < 0xFF7; i = fatlist[i], count++)
    {
        fs.seekg(_BLOCK_SIZE * _DATA_ST + fat_bias(i), ios::beg);
        fs.read(blc_buf, _BLOCK_SIZE);
        memcpy(data + count * _BLOCK_SIZE, blc_buf, _BLOCK_SIZE);
    }
    return data;
}

fatnum *get_fatlist(char *bytes)
{
    int ft_size = _BLOCK_SIZE * 9;
    fatnum *res = new fatnum[ft_size * 2 / 3];

    for (int i = 0, j = 0; i < ft_size; i += 3, j += 2)
    {
        // res[j] = static_cast<ushort>((bytes[i + 1] & 0x0F) << 8) | bytes[i];
        res[j] = ((bytes[i + 1] & 0x0F) << 8) | bytes[i];
        res[j + 1] = (bytes[i + 2] << 4) | ((bytes[i + 1] >> 4) & 0x0F);
    }

    return res;
}

int parse_cmd(string &line, string &cmd, string &opt, string &param)
{
    string word;
    cmd = "";
    opt = "";
    param = "";
    int l = line.length();
    const char *p = line.c_str();
    p = next_word(p, l, word);
    if (word.compare("ls") == 0)
    {
        cmd = "ls";
        while (l != 0)
        {
            p = next_word(p, l, word);
            if (word[0] == '-')
            {
                if (word.length() == 1)
                    return _WRONG_OPTION;
                else
                { //judge if multi options are all the same;
                    for (int i = 1; i < word.length(); i++)
                    {
                        if (word[i] != 'l')
                            return _WRONG_OPTION;
                    }
                    opt = "-l";
                }
            }
            else
            {
                if (param.length != 0)
                    return _TOO_MUCH_PARAM; //param should occur only  once;
                else
                {
                    param = word;
                }
            }
        }
        return _PARSE_SUCCESS;
    }
    else if (word.compare("cat") == 0)
    {
        cmd = "cat";
        if (l == 0)
            return _NO_PARAM;

        p = next_word(p, l, word);
        if (l != 0)
            return _TOO_MUCH_PARAM;
        else
        {
            param = word;
            return _PARSE_SUCCESS;
        }
    }
    else if (word.compare("quit") == 0)
    {
        cmd = "quit";
        if (l != 0)
            return _TOO_MUCH_PARAM;
        else
            return _PARSE_SUCCESS;
    }
    return _WRONG_COMMAND;
}

const char *next_word(const char *st, int &len, string &word)
{
    if (len == 0)
        return st;
    else
    {
        const char *ed = find(st, st + len, ' ');
        word = string(st, ed);
        while (ed != st + len && *ed == ' ') // skip multiple space
            ed++;
        len -= (ed - st);
        return ed + 1;
    }
}

void plain_print(DirNode *node)
{
    asm_prints(node->filename, _WHITE_COLOR);
    asm_prints(":\n", _WHITE_COLOR);
    DirNode *child;
    for (int i = 0; i < node->chd_ct; i++)
    {
        child = (node->children) + i;
        asm_prints(child->filename, child->is_dir ? _RED_COLOR : _WHITE_COLOR);
        if(i<node->chd_ct - 1)
            asm_prints("  ", _WHITE_COLOR);
    }
    asm_prints("\n", _WHITE_COLOR);
}
void detailed_print(DirNode *node)
{
    
}

Fat12Header &parse_header(byte *header)
{
    int idx = 0;
    Fat12Header res;
    memcpy(res.BS_OEMName, header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_BytsPerSec = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_SecPerClus = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_SecPerClus = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_NumFATs = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_DirEntCnt = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_TotSec16 = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_Media = *(header + hatrofs[idx]);
    idx++;

    res.BPB_FATSz16 = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_SecPerTrk = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_NumHeads = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_HiddSec = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_TotSec32 = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BS_DrvNum = *(header + hatrofs[idx]);
    idx++;

    res.BS_Reserved1 = *(header + hatrofs[idx]);
    idx++;

    res.BS_BootSig = *(header + hatrofs[idx]);
    idx++;

    res.BS_VolID = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    memcpy(res.BS_VolLab, header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    memcpy(res.BS_FileSysType, header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    return res;
}

DirEntry &parse_entry(byte *entry)
{
    int idx = 0;
    DirEntry res;
    memcpy(res.dirname, entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res.dirattr = *(entry + deatrofs[idx++]);

    memcpy(res.reserve, entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res.wrttime = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res.wrtdate = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res.fstclus = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res.filesize = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    return res;
}

number bs2n(byte *bytes, int n)
{
    number res = 0;
    const short base = 256;
    for (int i = n - 1; i >= 0; i--)
    {
        byte b = bytes[i];
        res *= base;
        number bit = 0;
        for (int j = 7; j >= 0; j--)
        {
            bit *= 2;
            if (b & 0x10)
            {
                bit++;
            }
            b = b << 1;
        }
        res += bit;
    }
    return res;
}

void hint()
{
    cout << "Wrong command. Please retry." << endl;
}
