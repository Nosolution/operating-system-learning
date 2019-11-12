#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include <stack>
#include <queue>
#include "includes.h"
#define _LS "ls"
#define _CAT "cat"
#define _EXIT "exit"
#define _LOAD_SUCCESS 0
#define _LOAD_FAILURE 1
#define _PARSE_SUCCESS 2
#define _WRONG_COMMAND 3
#define _WRONG_OPTION 4
#define _TOO_MUCH_PARAM 5
#define _NO_PARAM 6
#define _WHITE_COLOR 0
#define _RED_COLOR 1

using namespace std;

const char *no_such_file = "No such a file.\n";
const char *no_such_dir = "No such a directory.\n";
const char *not_file = "Not a file.\n";
const char *bye = "Bye\n";
const char *dir_end = ":\n";
const char *lf = "\n";
const char *space = " ";
const char *slash = "/";

fatnum *get_fatlist(char *bytes);
DirNode *read_meta_data(DirNode *node, const fatnum *fatlist, ifstream &fs);
byte *read_data(const number fstclus, const fatnum *fatlist, ifstream &fs, number &size);
DirNode *build_node(const DirEntry *entry);
bool is_normal_char(const char c);
int parse_cmd(const string &line, string &cmd, string &opt, string &param);
const char *next_word(const char *st, int &len, string &word);
void plain_print(DirNode *node);
void detailed_print(DirNode *node);
void print_abs_path(DirNode *node);
extern "C" void prints(const char *s, int color);  //print str in color
extern "C" void printi(const int i);               //print number
extern "C" void printcs(const char *s, int count); //print str in size
// void prints(const char *s, int color);  //print str in color
// void printi(const int i);               //print number
// void printcs(const char *s, int count); //print str in size
void warn();

int main()
{
    ifstream img;
    img.open("data.img", ios::binary);
    if (!img.is_open())
    {
        char open_failure_msg[25] = "Cannot open the image.\n";
        prints(open_failure_msg, _WHITE_COLOR);
        return 0;
    }
    //read header
    char header_bytes[_BLOCK_SIZE];
    img.seekg(_BLOCK_SIZE * _BPB_ST, ios::beg);
    img.read(header_bytes, _BLOCK_SIZE);
    Fat12Header *header = parse_header((const byte *)header_bytes);
    //read fatlist
    char fat_bytes[_BLOCK_SIZE * 9];
    img.seekg(_BLOCK_SIZE * _FAT_ST, ios::beg);
    img.read(fat_bytes, _BLOCK_SIZE * 9);
    fatnum *fatlist = get_fatlist(fat_bytes);
    //root name is under determination
    DirNode *root = new DirNode{"", true, 0};
    char entry_buf[_DIR_ITEM_SIZE];
    img.seekg(_BLOCK_SIZE * _ROOT_ENTRY_ST, ios::beg);
    //because data of files/subdirs in root are not stored in data area, need to manually call function
    for (unsigned int i = 0; i < header->BPB_DirEntCnt; i++)
    {
        img.read(entry_buf, _DIR_ITEM_SIZE);
        if (!is_normal_char(entry_buf[0]))
            break;
        DirEntry *entry = parse_entry((byte *)entry_buf);
        DirNode *node = build_node(entry);
        if (node->is_dir)
            read_meta_data(node, fatlist, img);
        root->add_child(node);
    }

    string line, cmd, opt, param;
    int code = 0;
    while (true)
    { //loop until quit command are input
        getline(cin, line);
        code = parse_cmd(line, cmd, opt, param);
        if (code == _PARSE_SUCCESS)
        {
            if (cmd.compare(_EXIT) == 0)
            {
                prints(bye, _WHITE_COLOR);
                break;
            }
            else if (cmd.compare(_LS) == 0)
            {
                DirNode *cur = root->find(param.c_str());
                if (cur == nullptr)
                {
                    prints(no_such_dir, _WHITE_COLOR);
                    continue;
                }
                else
                {
                    void (*print_func)(DirNode * node);
                    print_func = opt.length() > 0 ? detailed_print : plain_print;

                    DirNode *node, *tmp;
                    queue<DirNode *> node_q;
                    node_q.push(cur);
                    while (!node_q.empty())
                    { //print by bfs
                        node = node_q.front();
                        print_func(node);
                        prints(lf, _WHITE_COLOR);
                        for (unsigned int i = 0; i < node->chd_ct; i++)
                        {
                            tmp = node->children[i];
                            if (strcmp(tmp->name, ".") == 0 || strcmp(tmp->name, "..") == 0 || !tmp->is_dir)
                                continue;
                            node_q.push(tmp);
                        }
                        node_q.pop();
                    }
                }
            }
            else if (cmd.compare(_CAT) == 0)
            {
                DirNode *node = root->find(param.c_str());
                if (node == nullptr)
                {
                    prints(no_such_file, _WHITE_COLOR);
                }
                else if (node->is_dir)
                {
                    prints(not_file, _WHITE_COLOR);
                }
                else
                {
                    DirEntry *entry = ((DirEntry *)node->entry);
                    number size = 0;
                    byte *data = read_data(entry->fstclus, fatlist, img, size);
                    printcs(const_cast<char *>((char *)data), entry->filesize);
                    delete[] data;
                }
            }
        }
        else
        {
            warn();
        }
        prints(lf, _WHITE_COLOR);
    }
    img.close();
    return 0;
};

/**
 * Read meta data(meaning reading information of every dir entry instead of file data)
 * @param   node    the node that read data will be stored in
 * @param   fatlist linked list of cluster numbers
 * @param   fs      stream containing corresponding bytes
 * @return  also param1  
 */
DirNode *read_meta_data(DirNode *node, const fatnum *fatlist, ifstream &fs)
{
    if (node->is_dir && strcmp(node->name, ".") != 0 && strcmp(node->name, "..") != 0)
    {
        DirEntry *entry = (DirEntry *)node->entry;
        number size = 0;
        byte *data = read_data(entry->fstclus, fatlist, fs, size);
        byte entry_bytes[_DIR_ITEM_SIZE];
        unsigned int count = 0;

        while (count * _DIR_ITEM_SIZE < size)
        {
            memcpy(entry_bytes, data + count * _DIR_ITEM_SIZE, _DIR_ITEM_SIZE);
            if (!is_normal_char(entry_bytes[0]))
                break;
            DirEntry *chd_entry = parse_entry(entry_bytes);
            DirNode *child = build_node(chd_entry);

            read_meta_data(child, fatlist, fs); //dfs

            node->add_child(child);

            count++;
        }
        delete[] data;
    }
    return node;
};

/**
 * Read binary data by given fat image info.
 * Need to free the allocated memory manually. 
 * @param   fstclus     starting cluster number of fat
 * @param   fatlist     linked list of cluster numbers
 * @param   fs          stream containing corresponding bytes
 * @param   size        will be setthe number of bytes in calling
 * @return  gotten bytes, in heap
 */
byte *read_data(const number fstclus, const fatnum *fatlist, ifstream &fs, number &size)
{
    size = 0;
    for (number i = fstclus; i < _FAT_END; i = fatlist[i])
        size += _BLOCK_SIZE;

    byte *data = new byte[size];
    char blc_buf[_BLOCK_SIZE];
    int count = 0;
    int pos = fs.tellg(); //save the position of current stream
    for (number i = fstclus; i < _FAT_END; i = fatlist[i], count++)
    {
        fs.seekg(_BLOCK_SIZE * (_DATA_ST + fat_bias(i)), ios::beg);
        fs.read(blc_buf, _BLOCK_SIZE);
        memcpy(data + count * _BLOCK_SIZE, blc_buf, _BLOCK_SIZE);
    }
    fs.seekg(pos);
    return data;
};

/**
 * Construct dirnode by direntry
 * @param   entry   direntry containing the  information
 * @return  constructed node
 */
DirNode *build_node(const DirEntry *entry)
{

    char str[12];
    memcpy(str, entry->dirname, 11);
    int idx = 0;
    for (int i = 7; i >= 0; i--)
    {
        if (str[i] != ' ')
        {
            idx = i + 1;
            break;
        }
    }
    if (str[8] != ' ')
    {
        str[idx++] = '.';
        for (int i = 8; (i < 11) && (str[i] != ' '); idx++, i++)
        {
            str[idx] = str[i];
        }
    }
    str[idx] = '\0';
    DirNode *node = new DirNode{str, (entry->dirattr & DIRECTORY) != 0, entry->filesize};
    node->entry = (void *)entry;
    return node;
};

/**
 * Parse the linked list of cluster number for fat binary data
 * @param   bytes   fat table
 * @return  linked list represented by fatnum array
 */
fatnum *get_fatlist(char *bytes)
{
    int ft_size = _BLOCK_SIZE * 9;
    fatnum *res = new fatnum[ft_size * 2 / 3];

    for (int i = 0, j = 0; i < ft_size; i += 3, j += 2)
    {
        res[j] = ((bytes[i + 1] << 8) | bytes[i]) & 0x0FFF;
        res[j + 1] = ((bytes[i + 2] << 4) | ((bytes[i + 1] >> 4) & 0x0F)) & 0x0FFF;
    }

    return res;
};

/**
 * Judge if the char is printable.
 * @param   c   char to be judged
 * @return  true if the char is printable, false otherwise
 */
bool is_normal_char(const char c)
{
    return ((c & 0x80) == 0) && (32 <= c) && (c <= 126);
}

/**
 * Parse command from given str. For now only 'ls', 'cat' and 'quit' commands are supoorted.
 * @param   line    raw str to be parsed
 * @param   cmd     storing location for parsed cmd
 * @param   opt     storing location for parsed option
 * @param   param   storing location for parsed param(no multi params for now)
 * @return  result of parsing represented by integer code. 
 */
int parse_cmd(const string &line, string &cmd, string &opt, string &param)
{
    string word;
    cmd = "";
    opt = "";
    param = "";
    int l = line.length();
    const char *p = line.c_str();
    p = next_word(p, l, word);
    if (word.compare(_LS) == 0)
    {
        cmd = _LS;
        while (l != 0)
        {
            p = next_word(p, l, word);
            if (word[0] == '-')
            {
                if (word.length() == 1)
                    return _WRONG_OPTION;
                else
                { //judge if multi options are all the same;
                    for (unsigned int i = 1; i < word.length(); i++)
                    {
                        if (word[i] != 'l')
                            return _WRONG_OPTION;
                    }
                    opt = "-l";
                }
            }
            else
            {
                if (param.length() != 0)
                    return _TOO_MUCH_PARAM; //param should occur only  once;
                else
                {
                    if (word.length() == 0)
                    {
                        param = "/";
                    }
                    else
                    {
                        param = (word[0] == '/' ? "" : "/") + word;
                    }
                }
            }
        }
        return _PARSE_SUCCESS;
    }
    else if (word.compare(_CAT) == 0)
    {
        cmd = _CAT;
        if (l == 0)
            return _NO_PARAM;

        p = next_word(p, l, word);
        if (l != 0)
            return _TOO_MUCH_PARAM;
        else
        {
            if (word.length() == 0)
            {
                param = "/";
            }
            else
            {
                param = (word[0] == '/' ? "" : "/") + word;
            }

            return _PARSE_SUCCESS;
        }
    }
    else if (word.compare(_EXIT) == 0)
    {
        cmd = _EXIT;
        if (l != 0)
            return _TOO_MUCH_PARAM;
        else
            return _PARSE_SUCCESS;
    }
    return _WRONG_COMMAND;
};

/**
 * Get next word from current char ptr and return the ptr pointed to the location of next word 
 * @param   st    starting char ptr
 * @param   len   length to search a word
 * @param   word  str var to store the word got
 * @return  The ptr pointing to the location of next word
 */
const char *next_word(const char *st, int &len, string &word)
{
    if (len == 0)
        return st;
    else
    {
        //consider starting from spaces
        const char *ed = st;
        while (ed != st + len && *ed == ' ')
            ed++;
        if (ed == st + len)
            return ed;
        //start from first non-space char
        st = ed;
        ed = find(st, st + len, ' ');
        word = string(st, ed);
        while (ed != st + len && *ed == ' ') // skip multiple space
            ed++;
        len -= (ed - st);
        return ed;
    }
};

/**
 * Print the simplified info of selected directory, including subdirs and fils in this dir.
 * @param   node    starting dir node
 */
void plain_print(DirNode *node)
{
    print_abs_path(node);
    prints(dir_end, _WHITE_COLOR);
    DirNode *child;
    for (unsigned int i = 0; i < node->chd_ct; i++)
    {
        child = node->children[i];
        // cout << child->name << endl;
        prints(child->name, child->is_dir ? _RED_COLOR : _WHITE_COLOR);
        if (i < (node->chd_ct - 1))
        {
            prints(space, _WHITE_COLOR);
            prints(space, _WHITE_COLOR);
        }
    }
    prints(lf, _WHITE_COLOR);
};

/**
 * Print the detailed info of selected directory, including subdirs and fils in this dir with number of files in it or size of the file.
 * @param   node    starting dir node
 */
void detailed_print(DirNode *node)
{
    print_abs_path(node);
    printcs(space, 1);
    printi(node->count_subdir());
    printcs(space, 1);
    printi(node->count_subfile());
    printcs(dir_end, 2);

    DirNode *child;
    for (unsigned int i = 0; i < node->chd_ct; i++)
    {
        child = node->children[i];
        prints(child->name, child->is_dir ? _RED_COLOR : _WHITE_COLOR);
        if (strcmp(child->name, ".") != 0 && strcmp(child->name, "..") != 0)
        {
            if (child->is_dir)
            {
                prints(space, _WHITE_COLOR);
                printi(child->count_subdir());
                prints(space, _WHITE_COLOR);
                printi(child->count_subfile());
            }
            else
            {
                prints(space, _WHITE_COLOR);
                printi(child->size);
            }
        }
        prints(lf, _WHITE_COLOR);
    }
};

/**
 * Print the absolute path of the directory.
 * @param   node    starting dir node
 */
void print_abs_path(DirNode *node)
{
    if (!node->is_dir)
        return;
    stack<DirNode *> node_stk;
    DirNode *tmp = node;
    while (tmp != nullptr)
    {   //find parents and record the node sequence
        node_stk.push(tmp);
        tmp = tmp->parent;
    }
    while (!node_stk.empty())
    {  
        tmp = node_stk.top();
        node_stk.pop();
        prints(tmp->name, _WHITE_COLOR);
        prints(slash, _WHITE_COLOR);
    }
};

// void prints(const char *s, int color)
// {
//     printf("%s", s);
// }
// void printi(const int i)
// {
//     printf("%d", i);
// }
// void printcs(const char *s, int count)
// {
//     for (int i = 0; i < count; i++)
//         printf("%c", *(s + i));
// }

/**
 * Print warning when errors occur.
 */
void warn()
{
    char warning_msg[30] = "Wrong command. Please retry.\n";
    prints(warning_msg, _WHITE_COLOR);
};
