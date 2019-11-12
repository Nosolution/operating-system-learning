#include "dirnode.h"

DirNode::DirNode()
{
    name = new char[0];
    is_dir = false;
    entry = nullptr;
    size = 0;
    parent = nullptr;
    chd_ct = 0;
    children = nullptr;
};

DirNode::DirNode(const char *nm, bool id, unsigned int sz)
{
    int l = strlen(nm);
    name = new char[l];
    memcpy((void *)name, nm, l);
    is_dir = id;
    entry = nullptr;
    size = sz;
    parent = nullptr;
    chd_ct = 0;
    children = nullptr;
};

/**
 * Add a child to current node
 * @param   node    the node will add a child
 * @return  node itself
 */
DirNode &DirNode::add_child(DirNode *node)
{
    if (chd_ct == 0)
    {
        this->children = new DirNode *[30];
        children[0] = node;
    }
    else
    {
        children[chd_ct] = node;
    }
    chd_ct++;
    node->parent = this;
    return *this;
};

/**
 * Find the end node in current tree by given path
 * @param   path    the directory path(relative to current node)
 * @return  the found end node of nullptr
 */
DirNode *DirNode::find(const char *path)
{
    const char *ed = std::find(path, path + strlen(path), '/');
    if (strlen(name) != ed - path)
        return nullptr;
    else
    {
        for (int i = 0; i < ed - path; i++)
        {
            if (name[i] != path[i]) //if current dir is not the top dir
                return nullptr;
        }
        if (std::strlen(path) == 0 || (ed - path) >= (std::strlen(path) - 1)) // dir/ or dir
            return this;
        else //search in children
        {
            DirNode *res;
            for (unsigned int i = 0; i < chd_ct; i++)
            {
                DirNode *child = children[i];
                if (std::strcmp(child->name, ".") == 0 || std::strcmp(child->name, "..") == 0)
                    continue; //skip the parent dir and current dir

                res = child->find(ed + 1);
                if (res != nullptr)
                    return res;
            }
            return nullptr;
        }
    }
};

/**
 * Count the number of children which are directories
 * @return  number of directory children
 */
int DirNode::count_subdir()
{
    if (!this->is_dir)
        return 0;
    int r = 0;
    DirNode *child;
    for (unsigned int i = 0; i < chd_ct; i++)
    {
        child = children[i];
        if (strcmp(child->name, ".") == 0 || strcmp(child->name, "..") == 0)
            continue;
        if (child->is_dir)
            r++;
    }
    return r;
};

/**
 * Count the number of children which are files
 * @return  number of file children
 */
int DirNode::count_subfile()
{
    if (!this->is_dir)
        return 0;
    int r = 0;
    DirNode *child;
    for (unsigned int i = 0; i < chd_ct; i++)
    {
        child = children[i];
        if (strcmp(child->name, ".") == 0 || strcmp(child->name, "..") == 0)
            continue;
        if (!child->is_dir)
            r++;
    }
    return r;
};
