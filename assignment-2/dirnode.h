#include <cstdlib>
#include <cstring>
#include <algorithm>

class DirNode
{
    public:
        char* filename;
        bool is_dir;
        void* entry;
        int size;
        int chd_ct;
        DirNode* children;
        DirNode* parent;

        DirNode()
        {
            filename = "";
            is_dir = false;
            entry = nullptr;
            size = 0;
            parent = nullptr;
            chd_ct = 0;
            children = nullptr;
        };

        DirNode(char* name, bool id, int sz)
        {
            filename = name;
            is_dir = id;
            entry = nullptr;
            size = sz;
            parent = nullptr;
            chd_ct = 0;
            children = nullptr;

            if(id)
            {
                add_child(&parent_node());
                add_child(&cur_node());
                size = 0;
            }
        }

        DirNode& add_child(DirNode* node);

        DirNode* find(const char* name);

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

