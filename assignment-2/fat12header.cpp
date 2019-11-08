#include <cstring>
#include "fat12header.h"


/**
 * Parse fat12header from bytes
 * @param   header      bytes storing info of an header
 * @return  fat12header that parsed info   
 */
Fat12Header parse_header(const byte *header)
{
    int idx = 0;
    Fat12Header res;
    memcpy(res.BS_OEMName, header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_BytsPerSec = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_SecPerClus = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_SecPerClus = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_NumFATs = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_DirEntCnt = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_TotSec16 = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_Media = *(header + hatrofs[idx]);
    idx++;

    res.BPB_FATSz16 = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_SecPerTrk = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_NumHeads = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_HiddSec = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BPB_TotSec32 = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    res.BS_DrvNum = *(header + hatrofs[idx]);
    idx++;

    res.BS_Reserved1 = *(header + hatrofs[idx]);
    idx++;

    res.BS_BootSig = *(header + hatrofs[idx]);
    idx++;

    res.BS_VolID = bs2n(header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    memcpy(res.BS_VolLab, header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    memcpy(res.BS_FileSysType, header + hatrofs[idx], hatrofs[idx + 1] - hatrofs[idx]);
    idx++;

    return res;
};