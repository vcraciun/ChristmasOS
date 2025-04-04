#ifndef __FATFS__
#define __FATFS__

#include "types.h"
#include "TextArea.h"

#define _MCU_ENDIAN        1
/* The _MCU_ENDIAN defines which access method is used to the FAT structure.
/  1: Enable word access.
/  2: Disable word access and use byte-by-byte access instead.*/

#define _FS_READONLY    0
/* Setting _FS_READONLY to 1 defines read only configuration. This removes
/  writing functions, f_write, f_sync, f_unlink, f_mkdir, f_chmod, f_rename
/  and useless f_getfree. */

#define _FS_MINIMIZE    0
/* The _FS_MINIMIZE option defines minimization level to remove some functions.
/  0: Full function.
/  1: f_stat, f_getfree, f_unlink, f_mkdir, f_chmod and f_rename are removed.
/  2: f_opendir and f_readdir are removed in addition to level 1.
/  3: f_lseek is removed in addition to level 2. */

#define _DRIVES        10
/* Number of logical drives to be used. This affects the size of internal table. */

#define    _USE_MKFS    1
/* When _USE_MKFS is set to 1 and _FS_READONLY is set to 0, f_mkfs function is enabled. */

#define    _MULTI_PARTITION    0
/* When _MULTI_PARTITION is set to 0, each logical drive is bound to same
/  physical drive number and can mount only 1st primaly partition. When it is
/  set to 1, each logical drive can mount a partition listed in Drives[]. */

#define _USE_FSINFO    1
/* To enable FSInfo support on FAT32 volume, set _USE_FSINFO to 1. */

#define    _USE_SJIS    1
/* When _USE_SJIS is set to 1, Shift-JIS code transparency is enabled, otherwise
/  only US-ASCII(7bit) code can be accepted as file/directory name. */

#define    _USE_NTFLAG    1
/* When _USE_NTFLAG is set to 1, upper/lower case of the file name is preserved.
/  Note that the files are always accessed in case insensitive. */

/* Definitions corresponds to multiple sector size (not tested) */
#define    S_MAX_SIZ    512            /* Do not change */

#if S_MAX_SIZ > 512
#define    S_SIZ    (fs->s_size)
#else
#define    S_SIZ    512
#endif

/* File system object structure */
typedef struct _FATFS {
    WORD     id;                /* File system mount ID */
    WORD     n_rootdir;         /* Number of root directory entries */
    DWORD    winsect;           /* Current sector appearing in the win[] */
    DWORD    sects_fat;         /* Sectors per fat */
    DWORD    max_clust;         /* Maximum cluster# + 1 */
    DWORD    fatbase;           /* FAT start sector */
    DWORD    dirbase;           /* Root directory start sector (cluster# for FAT32) */
    DWORD    database;          /* Data start sector */
#if !_FS_READONLY
    DWORD    last_clust;        /* Last allocated cluster */
    DWORD    free_clust;        /* Number of free clusters */
#if _USE_FSINFO
    DWORD    fsi_sector;        /* fsinfo sector */
    BYTE     fsi_flag;          /* fsinfo dirty flag (1:must be written back) */
    BYTE     pad2;
#endif
#endif
    BYTE     fs_type;           /* FAT sub type */
    BYTE     sects_clust;       /* Sectors per cluster */
#if S_MAX_SIZ > 512
    WORD     s_size;            /* Sector size */
#endif
    BYTE     n_fats;            /* Number of FAT copies */
    BYTE     drive;             /* Physical drive number */
    BYTE     winflag;           /* win[] dirty flag (1:must be written back) */
    BYTE     pad1;
    char     volname[12];
    BYTE     win[S_MAX_SIZ];    /* Disk access window for Directory/FAT */
} FATFS;

/* Directory object structure */
typedef struct _DIR {
    WORD     id;             /* Owner file system mount ID */
    WORD     index;          /* Current index */
    FATFS* fs;             /* Pointer to the owner file system object */
    DWORD    sclust;         /* Start cluster */
    DWORD    clust;          /* Current cluster */
    DWORD    sect;           /* Current sector */
} DIR;

/* File object structure */
typedef struct _FIL {
    WORD     id;                /* Owner file system mount ID */
    BYTE     flag;              /* File status flags */
    BYTE     sect_clust;        /* Left sectors in cluster */
    FATFS* fs;                /* Pointer to the owner file system object */
    DWORD    fptr;              /* File R/W pointer */
    DWORD    fsize;             /* File size */
    DWORD    org_clust;         /* File start cluster */
    DWORD    curr_clust;        /* Current cluster */
    DWORD    curr_sect;         /* Current sector */
#if _FS_READONLY == 0
    DWORD    dir_sect;          /* Sector containing the directory entry */
    BYTE* dir_ptr;           /* Ponter to the directory entry in the window */
#endif
    BYTE     buffer[S_MAX_SIZ]; /* File R/W buffer */
} FIL;

/* File status structure */
typedef struct _FILINFO {
    DWORD fsize;               /* Size */
    WORD fdate;                /* Date */
    WORD ftime;                /* Time */
    BYTE fattrib;              /* Attribute */
    char fname[8 + 1 + 3 + 1];       /* Name (8.3 format) */
    char* long_fname;          /* Long Fname */
} FILINFO;

/* Definitions corresponds to multi partition */
#if _MULTI_PARTITION != 0    /* Multiple partition cfg */
typedef struct _PARTITION {
    BYTE pd;    /* Physical drive # (0-255) */
    BYTE pt;    /* Partition # (0-3) */
} PARTITION;
extern const PARTITION Drives[];       /* Logical drive# to physical location conversion table */
#define LD2PD(drv) (Drives[drv].pd)    /* Get physical drive# */
#define LD2PT(drv) (Drives[drv].pt)    /* Get partition# */

#else                                  /* Single partition cfg */

#define LD2PD(drv) (drv)        /* Physical drive# is equal to logical drive# */
#define LD2PT(drv) 0            /* Always mounts the 1st partition */

#endif


/* File function return code (FRESULT) */
typedef enum {
    FR_OK = 0,             /* 0 */
    FR_NOT_READY,          /* 1 */
    FR_NO_FILE,            /* 2 */
    FR_NO_PATH,            /* 3 */
    FR_INVALID_NAME,       /* 4 */
    FR_INVALID_DRIVE,      /* 5 */
    FR_DENIED,             /* 6 */
    FR_EXIST,              /* 7 */
    FR_RW_ERROR,           /* 8 */
    FR_WRITE_PROTECTED,    /* 9 */
    FR_NOT_ENABLED,        /* 10 */
    FR_NO_FILESYSTEM,      /* 11 */
    FR_INVALID_OBJECT,     /* 12 */
    FR_MKFS_ABORTED        /* 13 */
} FRESULT;

#define FILE_DELETED     0xE5
#define disk_initialize(a) 0

//#define disk_read(a,b,c,d) DISK::ReadSectors((dword)b,c,d);b+=d
//#define disk_write(a,b,c,d) DISK::WriteSectors((dword)b,c,d)

int disk_read(int drive, void* buffer, dword sector, dword count);
int disk_write(int drive, void* buffer, dword sector, dword count);

class FSFAT {
private:
    static FATFS* FatFs[_DRIVES];    /* Pointer to the file system objects (logical drives) */
    static WORD fsid;                /* File system mount ID */

    static FRESULT auto_mount(const char** path, FATFS** rfs, BYTE chk_wp);
    static DWORD get_fattime(void);
public:
    static FRESULT f_mount(BYTE, FATFS*);                        /* Mount/Unmount a logical drive */
    static FRESULT f_open(FIL*, const char*, BYTE, TEXTAREA*);              /* Open or create a file */
    static FRESULT f_read(FIL*, void*, DWORD, DWORD*, TEXTAREA*);             /* Read data from a file */
    static FRESULT f_write(FIL*, const void*, WORD, WORD*);      /* Write data to a file */
    static FRESULT f_lseek(FIL*, DWORD);                         /* Move file pointer of a file object */
    static FRESULT f_close(FIL*);                                /* Close an open file object */
    static FRESULT f_opendir(DIR*, const char*, TEXTAREA*);                 /* Open an existing directory  ------*/
    static FRESULT f_readdir(DIR*, FILINFO*);                    /* Read a directory item       ------*/
    static FRESULT f_stat(const char*, FILINFO*, TEXTAREA*);                /* Get file status */
    static FRESULT f_getfree(const char*, DWORD*, FATFS**);      /* Get number of free clusters on the drive */
    static FRESULT f_sync(FIL*);                                 /* Flush cached data of a writing file */
    static FRESULT f_unlink(const char*, TEXTAREA*);                        /* Delete an existing file or directory */
    static FRESULT f_mkdir(const char*, TEXTAREA*);                         /* Create a new directory */
    static FRESULT f_chmod(const char*, BYTE, BYTE, TEXTAREA*);             /* Change file/dir attriburte */
    static FRESULT f_rename(const char*, const char*, TEXTAREA*);           /* Rename/Move a file or directory */
    static FRESULT f_mkfs(BYTE, BYTE, BYTE);                     /* Create a file system on the drive */
};

/* File access control and file status flags (FIL.flag) */
#define    FA_READ                0x01
#define    FA_OPEN_EXISTING       0x00
#if _FS_READONLY == 0
#define    FA_WRITE               0x02
#define    FA_CREATE_NEW          0x04
#define    FA_CREATE_ALWAYS       0x08
#define    FA_OPEN_ALWAYS         0x10
#define FA__WRITTEN               0x20
#define FA__DIRTY                 0x40
#endif
#define FA__ERROR                 0x80

/* FAT sub type (FATFS.fs_type) */
#define FS_FAT12    1
#define FS_FAT16    2
#define FS_FAT32    3

/* File attribute bits for directory entry */
#define AM_RDO    0x01    /* Read only */
#define AM_HID    0x02    /* Hidden */
#define AM_SYS    0x04    /* System */
#define AM_VOL    0x08    /* Volume label */
#define AM_LFN    0x0F    /* LFN entry */
#define AM_DIR    0x10    /* Directory */
#define AM_ARC    0x20    /* Archive */

/* Offset of FAT structure members */
#define BS_jmpBoot            0
#define BS_OEMName            3
#define BPB_BytsPerSec        11
#define BPB_SecPerClus        13
#define BPB_RsvdSecCnt        14
#define BPB_NumFATs           16
#define BPB_RootEntCnt        17
#define BPB_TotSec16          19
#define BPB_Media             21
#define BPB_FATSz16           22
#define BPB_SecPerTrk         24
#define BPB_NumHeads          26
#define BPB_HiddSec           28
#define BPB_TotSec32          32
#define BS_55AA               510

#define BS_DrvNum             36
#define BS_BootSig            38
#define BS_VolID              39
#define BS_VolLab             43
#define BS_FilSysType         54

#define BPB_FATSz32           36
#define BPB_ExtFlags          40
#define BPB_FSVer             42
#define BPB_RootClus          44
#define BPB_FSInfo            48
#define BPB_BkBootSec         50
#define BS_DrvNum32           64
#define BS_BootSig32          66
#define BS_VolID32            67
#define BS_VolLab32           71
#define BS_FilSysType32       82

#define    FSI_LeadSig        0
#define    FSI_StrucSig       484
#define    FSI_Free_Count     488
#define    FSI_Nxt_Free       492

#define MBR_Table             446

#define    DIR_Name           0
#define    DIR_Attr           11
#define    DIR_NTres          12
#define    DIR_CrtTime        14
#define    DIR_CrtDate        16
#define    DIR_FstClusHI      20
#define    DIR_WrtTime        22
#define    DIR_WrtDate        24
#define    DIR_FstClusLO      26
#define    DIR_FileSize       28


/* Multi-byte word access macros  */

#if _MCU_ENDIAN == 1    /* Use word access */
#define    LD_WORD(ptr)         (WORD)(*(WORD*)(BYTE*)(ptr))
#define    LD_DWORD(ptr)        (DWORD)(*(DWORD*)(BYTE*)(ptr))
#define    ST_WORD(ptr,val)    *(WORD*)(BYTE*)(ptr)=(WORD)(val)
#define    ST_DWORD(ptr,val)   *(DWORD*)(BYTE*)(ptr)=(DWORD)(val)
#else
#if _MCU_ENDIAN == 2    /* Use byte-by-byte access */
#define    LD_WORD(ptr)        (WORD)(((WORD)*(BYTE*)((ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define    LD_DWORD(ptr)       (DWORD)(((DWORD)*(BYTE*)((ptr)+3)<<24)|((DWORD)*(BYTE*)((ptr)+2)<<16)|((WORD)*(BYTE*)((ptr)+1)<<8)|*(BYTE*)(ptr))
#define    ST_WORD(ptr,val)   *(BYTE*)(ptr)=(BYTE)(val); *(BYTE*)((ptr)+1)=(BYTE)((WORD)(val)>>8)
#define    ST_DWORD(ptr,val)  *(BYTE*)(ptr)=(BYTE)(val); *(BYTE*)((ptr)+1)=(BYTE)((WORD)(val)>>8); *(BYTE*)((ptr)+2)=(BYTE)((DWORD)(val)>>16); *(BYTE*)((ptr)+3)=(BYTE)((DWORD)(val)>>24)
#else
#error Do not forget to set _MCU_ENDIAN properly!
#endif
#endif

#endif /* _FATFS */
