#include "Disk.h"
#include "System.h"
#include "crt.h"
#include "Video.h"
#include "MemoryManager.h"

byte  DISK::disk_id;
dword DISK::partition_index;
byte *DISK::drivenrpartitions;
byte *DISK::drives;
PTABLE **DISK::partitions;
bool DISK::LOCKED;
FS_ACCESS_FUNCTIONS **DISK::diskpart;
FATFS DISK::filesystem;

void DISK::ResetDiskCache(void)
{
	memsetSSED((byte*)DISK_CACHE,0,CACHE_SIZE);
}

bool DISK::DiskExists(dword index)
{
	DoRmInt(0x13,0,0,0,index|0x80);
	if (*(byte*)RM_ERR>0 || *(byte*)(RM_AX+1)>0)
		return false;
	return true;
}

bool DISK::ChangeDisk(byte diskid)
{
	if (DiskExists(diskid))
	{
		disk_id=0x80|diskid;
		DoRmInt(0x13,0,0,0,disk_id);
		return true;
	}
	return false;
}

void DISK::ChangePartition(dword partitionid)
{
	partition_index=partitionid;
}

dword DISK::ReadSectors(dword addr,dword start_sector,dword count)
{
	dword code;

	*(word*)PACKET=0x10;
	*(word*)BLOCKS=(word)count;
	*(word*)OFFSET=0;
	*(word*)SEGMNT=(word)(DISK_CACHE>>4);
	*(dword*)MEMORY=start_sector;
	*(dword*)(MEMORY+4)=0;
	__asm{mov esi,PACKET}
	DoRmInt(0x13,0x4200,0,0,disk_id);
	memcpySSE((byte*)addr,(byte*)DISK_CACHE,count*512);
	code=*(byte*)(RM_AX+1);
	return code;
}

dword DISK::WriteSectors(dword addr,dword start_sector,dword count)
{
	dword code;

	memcpySSE((byte*)DISK_CACHE,(byte*)addr,count*512);
	*(word*)PACKET=0x10;
	*(word*)BLOCKS=(word)count;
	*(word*)OFFSET=0;
	*(word*)SEGMNT=(word)(DISK_CACHE>>4);
	*(dword*)MEMORY=start_sector; 
	*(dword*)(MEMORY+4)=0;
	__asm{mov esi,PACKET} 
	DoRmInt(0x13,0x4300,0,0,disk_id);
	code=*(byte*)(RM_AX+1);
	return code;
}

dword DISK::GetNumberOfPrimaryPartitions(dword addr)
{
	dword i;
	dword parts=0;

	for (i=0;i<4;i++)
		if (*(byte*)(addr+0x1BE+i*16+4)!=0)
			parts++;

	return parts;
}

dword DISK::GetCurrentDisk()
{
	return disk_id;
}

dword DISK::GetCurrentPartition()
{
	return partition_index;
}

void DISK::LockDiskAccess(void)
{
	LOCKED=true;
}

void DISK::UnlockDiskAccess(void)
{
	LOCKED=false;
}

bool DISK::IsDiskLocked(void)
{
	return LOCKED;
}

dword DISK::GetDiskStatus(void)
{
	dword err=0;

	DoRmInt(0x13,1,0,0,disk_id|0x80);
	err=*(byte*)(RM_AX+1);

	return err;
}

dword DISK::GetDiskTotalSectors(void)
{
	__asm{
		mov si,DRIVE_PARAMS
	}
	DoRmInt(0x13,0x4800,0,0,disk_id|0x80);
	return (*(dword*)(DRIVE_PARAMS+0x10));
}

dword DISK::GetPhysicalDrivesCount()
{
	dword error=0;
	dword did=0;

	while (!error)
	{
		DoRmInt(0x13,0,0,0,did|0x80);
		error=*(byte*)(RM_AX+1)|*(byte*)RM_ERR;
		did++;
	}

	return did-1;
}