#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>

class DirNode
{
public:
    const char *name;
    bool is_dir;
    void *entry;
    unsigned int size;
    unsigned int chd_ct;
    DirNode **children;
    DirNode *parent;

    DirNode();
    DirNode(const char *name, bool id, unsigned int sz);

    DirNode &add_child(DirNode *node);

    DirNode *find(const char *path);

    int count_subdir();
    int count_subfile();

    static DirNode parent_node(void)
    {
        DirNode node{".", true, 0};
        return node;
    };

    static DirNode cur_node(void)
    {
        DirNode node{"..", true, 0};
        return node;
    };
};
