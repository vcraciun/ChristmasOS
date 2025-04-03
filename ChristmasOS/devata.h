#ifndef __MYATA_H__
#define __MYATA_H__

#include "types.h"
#include "TextArea.h"
#include "pci.h"
#include "timer.h"

#define ATA_TIMEOUT 300000             // a large value should be better

// channel referes to primary or secondary
// we will populate while discovering
typedef struct chan
{
	word base_reg;
	word ctrl_reg;
	word bmide;
	byte nIEN;
}chan;

typedef struct partition
{
	byte boot_indicator;
	byte starting_head;
	word starting_sec_cyl;
	//word starting_cylinder:10;
	byte  system_id;
	byte  ending_head;
	word ending_sec_cyl;
	//word ending_cylinder:10;
	dword   start_lba;
	dword   total_sectors;
}  partition;

typedef struct mbr
{
	byte boot_code[446]; // 436 bytes of boot code + 10 bytes of Uniq ID of disk but all can be used for boot code
	partition partitions[4];      // 4 partitions
	word signature;     // 0x55,0xaa
}  mbr;      // TOTAL 446+4*16+2=446+64+2=512 bytes

// slots are end devices attached to either master or slave
typedef struct slot
{
	byte ps:1;      // primary 0, secondary 1
	byte ms:1;      // master =0 slave =1
	byte exists:1;  // if exists 1 else 0
	byte devtype:2; // unknown 000, 001 ata, 010 atapi, 011 sata, 100 satapi 
	byte lba:1;
	byte dma:1;
	chan *chanl;              // which channel this drive is connected primary or secondary
	word heads;    // number of heads 
	word sectors;  // number of sectors 
	dword cylinders;  // number of cylinders
	dword capacity;   // total number of sectors
	dword sectors28;
	unsigned long long sectors48;
	word drv_number; // drive number 0-4
	partition partition_table[4]; // a Partition table has 4 partitions
	struct slot *next;
}slot;


class DEVATA
{
public:
	DEVATA(pci_bus *pci_data,TEXTAREA *text_zone,TIMER *timer);
	~DEVATA();
	void display_partition_info(partition *p);

	void guess_fs_type();
	void display_sysdrive_info();
	dword read(dword block,byte *buffer); // block is relative to partition begining
	dword write(dword block,byte *buffer){};

private:
	void init_disks();
	slot *get_device(word type);
	dword ata_r_sector(dword block,word *buf);
	dword ata_w_sector(dword block,word *buf);

	bool search_disks(pci_bus *pci);
	bool pio_wait_busy(word port);
	bool pio_wait_busy_astat(word port);
	bool reset_controller(word port);
	bool detect_master(word port);
	bool detect_slave(word port);
	void browse_slots();
	void display_slot_info();
	dword ata_rw_sector(slot *drv,dword block,word *buf,byte direction);
	//void init_sysdrives();
	//void display_sysdrive_info();

	dword my_drives[4];
	chan channels[2];
	slot *slots;
	TEXTAREA *textarea;
	TIMER *tmr;
	pci_bus *pci;

	word part_num;
	byte fs_type;
	bool guessed;
	char name[5];

	class fat16 *fs;
	void set_partition(word p);
	word get_partition();
	void display_info();
	char *get_name();
	void set_fs_type(byte fst);
	char *fs_type_to_string(word fs_type);
};

#endif
