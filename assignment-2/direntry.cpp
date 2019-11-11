#include <cstring>
#include "direntry.h"

/**
 * Parse entry from bytes
 * @param   entry       bytes storing info of an direntry
 * @return  ptr to the direntry that parsed info   
 */
DirEntry *parse_entry(byte *entry)
{
    int idx = 0;
    DirEntry *res = new DirEntry;
    memcpy(res->dirname, entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res->dirattr = *(entry + deatrofs[idx++]);

    memcpy(res->reserve, entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res->wrttime = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res->wrtdate = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res->fstclus = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    res->filesize = bs2n(entry + deatrofs[idx], deatrofs[idx + 1] - deatrofs[idx]);
    idx++;

    return res;
};