#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include <stack>
#include <queue>
#include "includes.h"
#define _PARSE_SUCCESS 0
#define _WRONG_COMMAND 1
#define _WRONG_OPTION 2
#define _TOO_MUCH_PARAM 3
#define _NO_PARAM 4
#define _WHITE_COLOR 10
#define _RED_COLOR 12

using namespace std;

fatnum *get_fatlist(char *bytes);
DirNode &read_meta_data(DirNode &node, const fatnum *fatlist, ifstream &fs);
byte *read_data(const number fstclus, const fatnum *fatlist, ifstream &fs, number &size);
DirNode &build_node(const DirEntry &entry);
int parse_cmd(const string &line, string &cmd, string &opt, string &param);
const char *next_word(const char *st, int &len, string &word);
void bfs_print_node(DirNode *node, void (*print)(DirNode *node));
void plain_print(DirNode *node);
void detailed_print(DirNode *node);
void print_abs_path(DirNode *node);
void asm_prints(const char *s, int color);  //print str in color
void asm_printi(const int i);               //print number
void asm_printcs(const char *s, int count); //print str in size
void hint();

int main()
{
    ifstream img;
    img.open("data.img", ios::binary);
    //read header
    char header_bytes[_BLOCK_SIZE];
    img.seekg(_BLOCK_SIZE * _BPB_ST, ios::beg);
    img.read(header_bytes, _BLOCK_SIZE);
    Fat12Header &header = parse_header((const byte *)header_bytes);
    //read fatlist
    char fat_bytes[_BLOCK_SIZE * 9];
    img.seekg(_BLOCK_SIZE * _FAT_ST, ios::beg);
    img.read(fat_bytes, _BLOCK_SIZE * 9);
    fatnum *fatlist = get_fatlist(fat_bytes);
    //root name is under determination
    DirNode &root = *new DirNode{"", true, 0};
    char entry_buf[_DIR_ITEM_SIZE];
    img.seekg(_BLOCK_SIZE * _ROOT_ENTRY_ST, ios::beg);
    //because data of files/subdirs in root are not stored in data area, need to manually call function
    for (unsigned int i = 0; i < header.BPB_DirEntCnt; i++)
    {
        img.read(entry_buf, _DIR_ITEM_SIZE);
        if (entry_buf[0] == '\0')
            break;
        DirEntry &entry = parse_entry((byte *)entry_buf);
        DirNode &node = build_node(entry);
        if (node.is_dir)
            read_meta_data(node, fatlist, img);
        root.add_child(&node);

        // img.seekg(_DIR_ITEM_SIZE, ios::cur);
    }

    plain_print(&root);
    // cout << "Hello World!" << endl;
    string line, cmd, opt, param;
    int code;
    //loop until quit command are input
    while (true)
    {
        getline(cin, line);
        code = parse_cmd(line, cmd, opt, param);
        if (code == _PARSE_SUCCESS)
        {
            if (cmd.compare("quit") == 0)
            {
                cout << "bye" << endl;
                break;
            }
            else if (cmd.compare("ls") == 0)
            {
                char main_dir[60] = "";
                // root.name = "main"; //make method perform consistantly
                strcat(main_dir, param.c_str());
                DirNode *cur = root.find(main_dir);
                // root.name = "";

                void (*print_func)(DirNode * node);
                print_func = opt.length() > 0 ? detailed_print : plain_print;

                DirNode *node, *tmp;
                queue<DirNode *> node_q;
                node_q.push(cur);
                while (!node_q.empty())
                { //print by bfs
                    node = node_q.front();
                    print_func(node);
                    for (unsigned int i = 0; i < node->chd_ct; i++)
                    {
                        tmp = node->children + i;
                        if (strcmp(tmp->name, ".") == 0 || strcmp(tmp->name, "..") == 0 || !tmp->is_dir)
                            continue;
                        node_q.push(tmp);
                    }
                    node_q.pop();
                }
            }
            else if (cmd.compare("cat") == 0)
            {
                DirNode *node = root.find(param.c_str());
                DirEntry *entry = ((DirEntry *)node->entry);
                number size = 0;
                byte *data = read_data(entry->fstclus, fatlist, img, size);
                asm_printcs(const_cast<char *>((char *)data), entry->filesize);
                delete[] data;
            }
        }
        else
        {
            hint();
        }
        asm_prints("\n", _WHITE_COLOR);
    }
    return 0;
};

/**
 * Read meta data(meaning reading information of every dir entry instead of file data)
 * @param   node    the node that read data will be stored in
 * @param   fatlist linked list of cluster numbers
 * @param   fs      stream containing corresponding bytes
 * @return  also param1  
 */
DirNode &read_meta_data(DirNode &node, const fatnum *fatlist, ifstream &fs)
{
    if (node.is_dir && strcmp(node.name, ".") != 0 && strcmp(node.name, "..") != 0)
    {
        DirEntry *entry = (DirEntry *)node.entry;
        number size = 0;
        byte *data = read_data(entry->fstclus, fatlist, fs, size);
        byte entry_bytes[_DIR_ITEM_SIZE];
        unsigned int count = 0;

        while (count * _DIR_ITEM_SIZE < size)
        {
            memcpy(entry_bytes, data + count * _DIR_ITEM_SIZE, _DIR_ITEM_SIZE);
            if (entry_bytes[0] == '\0')
                break;
            DirEntry &chd_entry = parse_entry(entry_bytes);
            DirNode &child = build_node(chd_entry);

            read_meta_data(child, fatlist, fs); //dfs

            node.add_child(&child);
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
DirNode &build_node(const DirEntry &entry)
{

    char str[12];
    memcpy(str, entry.dirname, 11);
    for (int i = 10; i >= 0; i--)
    {
        if (str[i] != ' ')
        {
            str[i + 1] = '\0';
            break;
        }
    }
    DirNode &node = *new DirNode{str, (entry.dirattr & DIRECTORY) != 0, entry.filesize};
    node.entry = (void *)&entry;
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
        // res[j] = static_cast<ushort>((bytes[i + 1] & 0x0F) << 8) | bytes[i];
        res[j] = ((bytes[i + 1] & 0x0F) << 8) | bytes[i];
        res[j + 1] = (bytes[i + 2] << 4) | ((bytes[i + 1] >> 4) & 0x0F);
    }

    return res;
};

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
                    else if (word[0] != '/')
                    {
                        param = "/"+ word;
                    }
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
            if (word.length() == 0)
            {
                param = "/";
            }
            else if (word[0] != '/')
            {
                param = "/"+ word;
            }
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
    if (cmd.compare("quit") != 0)
    {
        
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
    asm_prints(":\n", _WHITE_COLOR);
    DirNode *child;
    for (unsigned int i = 0; i < node->chd_ct; i++)
    {
        child = (node->children) + i;
        asm_prints(child->name, child->is_dir ? _RED_COLOR : _WHITE_COLOR);
        if (i < node->chd_ct - 1)
            asm_prints("  ", _WHITE_COLOR);
    }
    asm_prints("\n", _WHITE_COLOR);
};

/**
 * Print the detailed info of selected directory, including subdirs and fils in this dir with number of files in it or size of the file.
 * @param   node    starting dir node
 */
void detailed_print(DirNode *node)
{
    print_abs_path(node);
    asm_prints(" ", _WHITE_COLOR);
    asm_printi(node->count_subdir());
    asm_prints(" ", _WHITE_COLOR);
    asm_printi(node->count_subfile());
    asm_prints(":\n", _WHITE_COLOR);

    DirNode *child;
    for (unsigned int i = 0; i < node->chd_ct; i++)
    {
        child = node->children + i;
        if (strcmp(child->name, ".") == 0 || strcmp(child->name, "..") == 0)
            continue;
        asm_prints(child->name, child->is_dir ? _RED_COLOR : _WHITE_COLOR);
        if (child->is_dir)
        {
            asm_prints(" ", _WHITE_COLOR);
            asm_printi(child->count_subdir());
            asm_prints(" ", _WHITE_COLOR);
            asm_printi(child->count_subfile());
        }
        else
        {
            asm_prints(" ", _WHITE_COLOR);
            asm_printi(child->size);
        }
        asm_prints("\n", _WHITE_COLOR);
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
    {
        node_stk.push(tmp);
        tmp = tmp->parent;
    }
    while (!node_stk.empty())
    {
        tmp = node_stk.top();
        node_stk.pop();
        asm_prints(tmp->name, _WHITE_COLOR);
        asm_prints("/", _WHITE_COLOR);
    }
};

void asm_prints(const char *s, int color)
{
    cout << s;
}
void asm_printi(const int i)
{
    cout << i;
}
void asm_printcs(const char *s, int count)
{
    for (int i = 0; i < count; i++)
        cout << *(s + i);
}

/**
 * Print command hint when errors occur.
 */
void hint()
{
    cout << "Wrong command. Please retry." << endl;
};
