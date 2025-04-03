#ifndef __DISK__
#define __DISK__

#include "types.h"
#include "Fat.h"

#define DISK_CACHE 0x50000
#define CACHE_SIZE 0x50000

#define DRIVE_PARAMS 0x2700

#define RM_ERR 0x801
#define RM_AX  0x802
#define RM_BX  0x804
#define RM_CX  0x806
#define RM_DX  0x808

#define PACKET 0x810
#define BLOCKS 0x812
#define OFFSET 0x814
#define SEGMNT 0x816
#define MEMORY 0x818

typedef BYTE DSTATUS;

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* Command code for disk_ioctrl() */
#define GET_SECTOR_COUNT	1
#define GET_SECTOR_SIZE		2
#define CTRL_SYNC			3
#define CTRL_POWER			4
#define CTRL_LOCK			5
#define CTRL_EJECT			6
#define MMC_GET_CSD			10
#define MMC_GET_CID			11
#define MMC_GET_OCR			12
#define ATA_GET_REV			20
#define ATA_GET_MODEL		21
#define ATA_GET_SN			22

typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


typedef struct _PTABLE{
	byte partition_state;
	byte begin_head;
	word begin_cs;
	byte partition_type;
	byte end_head;
	word end_cs;
	dword partition_start;
	dword nr_sectors;     
}PTABLE;    

typedef struct _FS_ACCESS_FUNCTIONS{
	FRESULT (*fs_mount)(BYTE, FATFS*);                        
	FRESULT (*fs_open)(FIL*, const char*, BYTE);              
	FRESULT (*fs_read)(FIL*, void*, WORD, WORD*);             
	FRESULT (*fs_write)(FIL*, const void*, WORD, WORD*);      
	FRESULT (*fs_lseek)(FIL*, DWORD);                         
	FRESULT (*fs_close)(FIL*);                                
	FRESULT (*fs_opendir)(DIR*, const char*);                 
	FRESULT (*fs_readdir)(DIR*, FILINFO*);                    
	FRESULT (*fs_stat)(const char*, FILINFO*);                
	FRESULT (*fs_getfree)(const char*, DWORD*, FATFS**);      
	FRESULT (*fs_sync)(FIL*);                                 
	FRESULT (*fs_unlink)(const char*);                        
	FRESULT (*fs_mkdir)(const char*);                         
	FRESULT (*fs_chmod)(const char*, BYTE, BYTE);             
	FRESULT (*fs_rename)(const char*, const char*);           
	FRESULT (*fs_mkfs)(BYTE, BYTE, BYTE); 
}FS_ACCESS_FUNCTIONS;

class DISK{
	private:
		static byte disk_id;
		static dword partition_index;

		static byte *drivenrpartitions;
		static byte *drives;
		static PTABLE **partitions;
		static char *currentpath;
		static bool LOCKED;
		static FATFS filesystem;		

	public:
		static void ResetDiskCache(void);
		static dword ReadSectors(dword addr,dword start_sector,dword count);
		static dword WriteSectors(dword addr,dword start_sector,dword count);
		static dword GetDiskStatus(void);
		static bool ChangeDisk(byte diskid);
		static void ChangePartition(dword partitionid);
		static bool DiskExists(dword index);
		static dword GetNumberOfPrimaryPartitions(dword addr);
		static dword GetDiskTotalSectors(void);
		static dword GetCurrentDisk();
		static dword GetCurrentPartition();
		static dword GetPhysicalDrivesCount();

		static void LockDiskAccess(void);
		static void UnlockDiskAccess(void);
		static bool IsDiskLocked(void);

		static FS_ACCESS_FUNCTIONS **diskpart;
};

#endif