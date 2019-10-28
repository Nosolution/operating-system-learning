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
    if(node->parent==nullptr)
    {
        node->parent[1];
    }
    node->parent = this;
    return *this;
};

DirNode* DirNode::find(const char* path)
{
    if(!this->is_dir)
        return nullptr;
    else
    {
        const char* ed = std::find(path, path+strlen(path), '/');
        if(strlen(filename)!=ed-path)
            return nullptr;
        else
        {
            for(int i= 0; i<ed-path;i++)
            {
                if(filename[i]!=path[i])//if filename is not the top dir
                    return nullptr;
            }
            if( ed-path >= strlen(path)-1 )// dir/ or dir
                return this;
            else    //search in children
            {
                DirNode* res;
                for(int i = 0;i < chd_ct; i++)
                {
                    DirNode* child = children + i;
                    if(std::strcmp(child->filename, ".")==0 || std::strcmp(child->filename, "..")==0)
                        continue;//skip the parent dir and current dir

                    res = child->find(ed + 1);
                    if(res!=nullptr)
                        return res;
                }
                return nullptr;
            }
            
        }
    }
    
};

int DirNode::count_subdir()
{
    if(!this->is_dir)
        return 0;
    int r = 0;
    DirNode* child;
    for(int i=0;i<chd_ct;i++)
    {
        child = children+i;
        if(strcmp(child->filename, ".")==0 || strcmp(child->filename, "..")==0)
            continue;
        if(child->is_dir)
            r++;
    }
    return r;
};
int DirNode::count_subfile()
{
    if(!this->is_dir)
        return 0;
    int r = 0;
    DirNode* child;
    for(int i=0;i<chd_ct;i++)
    {
        child = children+i;
        if(strcmp(child->filename, ".")==0 || strcmp(child->filename, "..")==0)
            continue;
        if(!child->is_dir)
            r++;
    }
    return r;
};


