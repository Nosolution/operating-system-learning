#include <cstdlib>
#include <cstring>
#include <algorithm>

class DirNode
{
    public:
        const char* filename;
        bool is_dir;
        void* entry;
        unsigned int size;
        unsigned int chd_ct;
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

        DirNode(const char* name, bool id, unsigned int sz)
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
                DirNode parent{".", true, 0};
                add_child(&parent);
                DirNode cur{"..", true, 0};
                add_child(&cur);
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

