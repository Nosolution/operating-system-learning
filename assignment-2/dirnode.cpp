#include "dirnode.h"

DirNode& DirNode::add_child(DirNode* node)
{
    if(chd_ct==0)
    {
        this->children[30];
        children[0] = *node;
    }
    else
    {
        children[chd_ct++] = *node;
    }
    chd_ct++;
    return *this;
};
