#include <cstdlib>
#include <cstring>
#include <algorithm>

class DirNode
{
    public:
        char* filename;
        bool is_dir;
        void* entry;
        int chd_ct;
        DirNode* children;

        DirNode()
        {
            filename = "";
            is_dir = false;
            entry = nullptr;
            chd_ct = 0;
            children = nullptr;
        };

        DirNode(char* name, bool id)
        {
            filename = name;
            is_dir = id;
            entry = nullptr;
            chd_ct = 0;
            children = nullptr;

            if(id)
            {
                add_child(&parent_node());
                add_child(&cur_node());
            }
        }

        DirNode& add_child(DirNode* node);

        DirNode* find(const char* name);

        static DirNode parent_node(void)
        {
                DirNode node{".", true};
                return node;
        };

        static DirNode cur_node(void)
        {
                DirNode node{"..", true};
                return node;
        };
};

