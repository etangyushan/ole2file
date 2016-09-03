/*
 * ole2.h
 *      解析 ole 2.0
 *  Created on: Sep 3, 2016
 *      Author: root
 */

#ifndef OLE2_H_
#define OLE2_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef unsigned long ULONG;       //4 bytes
typedef unsigned short USHORT;     //2 bytes
typedef short OFFSET;              //2 bytes
typedef ULONG SECT;                //4 bytes
typedef ULONG FSINDEX;             //4 bytes
typedef USHORT FSOFFSET;           //2 bytes
typedef ULONG DFSIGNATURE;         //4 bytes
typedef unsigned char BYTE;        //1 bytes
typedef unsigned short WORD;       //2 bytes
typedef unsigned long DWORD;       //4 bytes
typedef WORD DFPROPTYPE;           //2 bytes
typedef ULONG SID;                 //4 bytes
typedef BYTE CLSID[16];            //16 bytes
typedef CLSID GUID;

//8 bytes
typedef struct tagFILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
}FILETIME, TIME_T;

const SECT DIFSECT = 0xFFFFFFFC;
const SECT FATSECT = 0xFFFFFFFD;
const SECT ENDOFCHAIN = 0xFFFFFFFE;
const SECT FREESECT = 0xFFFFFFFF;

/*
 * @brief 复合文档头信息
 *
 * 复合文档头在文件的开始，且其大小必定为 512字节。
 * 这意味着第一个 sector的开始位置相对于文件的偏移量为 512 字节
 */
typedef struct ole2_header_s_
{
    BYTE _abSig[8];               // [000H,08] 复合文档的文件标识，
		                          // 当前的版本标识：{0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1}
		                          // 旧的版本标识：{0x0e, 0x11, 0xfc, 0x0d, 0xd0, 0xcf, 0x11, 0xe0}
    //uint8_t _clid[16];
    CLSID _clid;                  // [008H,16] 文件的唯一标识（不重要，可全部为0）

    USHORT _uMinorVersion;        // [018H,02] 文件格式修订号（一般为 0x003E）

    USHORT _uDllVersion;          // [01AH,02] 文件格式版本号（一般为 0x0003）

    USHORT _uByteOrder;           // [01CH,02] 字节顺序标识（一般为 0xFFFE）
		                          // 文件数据的二进制存储有两种方法Little-Endian 和 Big-Endian，
								  // 但实际应用中只使用Little-Endian方法即：
								  // 低位8字节存放在地址的低位，高位8字节存放在地址的高位。
								  // 例：一个32位的整数13579BDFH（转为十进制即324508639），
								  // 以Little-Endian存放为DFH 9BH 57H13H，
								  // 以Big-Endian 存放为 13H 57H 9BH DFH。（H下标表示十六进制）

    USHORT _uSectorShift;         // [01EH,02] 复合文档中的 sector 的大小(ssz) 以 2 的幂形式存储
	                              // 一般 _uSectorShift = 9
	                              // 即：2^9 = 512 ，sector 的一般大小为 512 字节

    USHORT _uMiniSectorShift;     // [020H,02] 复合文档中的 short-sector 的大小(ssz) 以 2 的幂形式存储
	                              // 一般 _uMiniSectorShift = 6
								  // 即：2^6 = 64 ，short-sector 的一般大小为 64 字节

    USHORT _usReserved;           // [022H,02] 未用,必须为0

	ULONG _ulReserved1;           // [024H,04] 未用,必须为0

	ULONG _ulReserved2;           // [028H,04] 未用,必须为0

	FSINDEX _csectFat;            // [02CH,04] 用于存放扇区配置表（SAT）的sector总数

	SECT _sectDirStart;           // [030H,04] 用于存放目录流的第一个sector的SID

	DFSIGNATURE _signature;       // [034H,04] 未用，必须是0

	ULONG _ulMiniSectorCutoff;    // [038H,04] 标准流的最小大小(一般为4096 bytes), 小于此值的流即为短流

	SECT _sectMiniFatStart;       // [03CH,04] 用于存放短扇区配置表（SSAT）的第一个sector的SID，
	                              // 等于 -2 （End Of Chain SID） 表示不存在

	FSINDEX _csectMiniFat;        // [040H,04] 用于存放短扇区配置表（SSAT）的sector总数

	SECT _sectDifStart;           // [044H,04] 用于存放主扇区配置表（MSAT）的第一个sector的SID，
	                              // 等于 -2 （End Of Chain SID） 表示无附加的sector

	FSINDEX _csectDif;            // [048H,04] 用于存放主扇区配置表（MSAT）的sector总数

	SECT _sectFat[109];           // [04CH,436] 存放主扇区的配置表（MSAT）的第一部分，包含前109个SID
} ole2_header_s;

/**
 * @brief 目录类型
 */
typedef enum tagSTGTY
{
     STGTY_INVALID  = 0,
     STGTY_STORAGE = 1,
     STGTY_STREAM = 2,
     STGTY_LOCKBYTES = 3,
     STGTY_PROPERTY = 4,
     STGTY_ROOT = 5,
} STGTY;

//目录颜色
typedef enum tagDECOLOR
{
     DE_RED = 0,
     DE_BLACK  = 1,
} DECOLOR;

/**
 * @brief 目录入口的信息
 *  一个目录入口的大小严格的为 128 字节，计算相对目录流的偏移量公式为
 *  dir_entry_pos(DID) = DID * 128
 *  DID:directory entry identifier
 *  目录ID都是顺序排列
 */
typedef struct ole2_direcotryentry_s_
{
     BYTE _abEleName[64];       // [000H,64] 目录名称(字符数组)，一般为16位的Unicode字符
                                // 以0结束（最大长度为31个字符）

     WORD _CbEleName;           // [040H,02] 目录名称长度用于存放名字的区域大小，包括结尾的0
                                // 如：一个名字有5个字符，则此值为 (5+1)*2 = 12

     STGTY _mse;                // [042H,01] 目录类别，入口类型:
	                            // 00H = Empty
	                            // 03H = LockBytes (unknown)
								// 01H = User storage
								// 04H = Property (unknown)
								// 02H = User stream
								// 05H = Root storage

     DECOLOR _bflags;           // [043H,01] 目录的节点颜色：
                                // 00H = Red
                                // 01H = Black

     SID _sidLeftSib_DID;       // [044H,04] 目录的左子节点的DID号（如果是 storage 或者 stream）
                                // 若没有左节点就为 -1

     SID _sidRightSib_DID;      // [048H,04] 目录的右子节点的DID号（如果是 storage 或者 stream）
                                // 若没有右节点就为 -1

     SID _sidChild_DID;         // [04CH,04] 目录的子节点的DID号(若此入口为storage), 其他为－1。

     //uint8_t _clsId[16];
     GUID _clsId;               // [050H,16] 唯一标识符(若此入口为storage，不重要可全为0),

     DWORD _dwUserFlags;        // [060H,04] 用户标识 (不重要可全为0 _mse=STGTY_STORAGE)

     //uint8_t _time[16];
     TIME_T _time[2];           // [064H,16] 目录的创建和修改时间 (大多情况不写 _mse=STGTY_STORAGE)

     SECT _sectStart;           // [074H,04] 若此为流的入口(如果 _mse=STGTY_STREAM)，
                                // 指定流的第一个sector或者short-sector的SID,
	                            // 若此为根仓库入口，指定短流存放流的第一个sector的SID,
								// 其他情况，为0

     ULONG _ulSize;             // [078H,04] 若此为流的入口(如果_mse=STGTY_STREAM)，指定流的大小（字节）
	                            // 若此为根仓库入口，指定短流存放流的大小（字节）
                                // 其他情况为0

     DFPROPTYPE _dptPropType;   // [07CH,02] 未使用,必须是0
} ole2_direcotryentry_s;


typedef struct ole2_s_
{
   FILE *fp; //文件句柄


} ole2_s;

#endif /* OLE2_H_ */
