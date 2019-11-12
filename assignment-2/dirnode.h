#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "commons.h"

class DirNode
{
public:
    const char *name;
    bool is_dir;
    void *entry;
    unsigned int size;      //file size, zero if is dir
    unsigned int chd_ct;    
    DirNode **children;
    DirNode *parent;

    DirNode();
    DirNode(const char *name, bool id, unsigned int sz);

    DirNode &add_child(DirNode *node);

    DirNode *find(const char *path);

    int count_subdir();
    int count_subfile();
};
