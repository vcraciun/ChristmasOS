//////////////////////////////////////////////////////////
// This file is a part of Nanos Copyright (C) 2008, 2009//
// ashok.s.das@gmail.com                                //
//////////////////////////////////////////////////////////
// IDE: Prototypes declaration                          //
//                                                      //
//////////////////////////////////////////////////////////
#ifndef __IDE_H__
#define __IDE_H__


#include "types.h"

#define ATA_BASE_PRI	0x1f0 	// default ISA
#define ATA_BASE_SEC	0x170	// default ISA
#define MASTER				0xA0
#define SLAVE				0xB0

// bellow are offset from ATA_BASE_xxx
#define DATA_REG	0	//RW
#define ERR_REG		1	//R
#define FEATURE_REG	1	//RW
#define SECT_CNT_REG	2	//RW
#define LBA_LOW_REG	3	//rw
#define LBA_MID_REG	4	//rw
#define LBA_HI_REG	5	//rw
#define DRV_HD_REG	6	//rw
#define STATUS_REG	7	//r
#define CMD_REG		7	//W
#define ALT_ST_REG	0x206	//R
#define DEV_CTRL_REG	0x206	//w 

//status register fields
#define STA_ERR		1<<0	//error
#define STA_IDX		1<<1	//index mark
#define STA_CORR	1<<2	//data corrected
#define STA_DRQ		1<<3	//Data Transfer Requested
#define STA_DSC		1<<4	//seek complete
#define STA_DF		1<<5	//Device Fault
#define STA_DRDY	1<<6	//device ready
#define STA_BSY		1<<7	//busy

//error register fields
#define ERR_AMNF	1<<0	//Address Mark Not Found
#define ERR_TK0NF	1<<1	//Track 0 Not Found
#define ERR_ABRT	1<<2	//command aborted
#define ERR_MCR		1<<3	//Media Change Requested
#define ERR_IDNF	1<<4	//ID mark Not Found
#define ERR_MC		1<<5	//Media Changed
#define ERR_UNC		1<<6	//Uncorrectable data error
#define ERR_BBK		1<<7	//Bad Block

/* Important bits in the device control register.
   See ATA/ATAPI-4 spec, section 7.9.6 */
#define ATA_CTL_SRST    0x04
#define ATA_CTL_nIEN    0x02

/* ATA command bytes */
#define	ATA_CMD_READ		0x20	/* read sectors */
#define	ATA_CMD_WRITE		0x30	/* write sectors */
#define	ATA_CMD_READ_EXT		0x24	/* read sectors */
#define	ATA_CMD_WRITE_EXT		0x34	/* write sectors */
#define	ATA_CMD_PKT		0xA0	/* signals ATAPI packet command */
#define	ATA_CMD_PID		0xA1	/* identify ATAPI device */
#define	ATA_CMD_READMULT	0xC4	/* read sectors, one interrupt */
#define	ATA_CMD_MULTMODE	0xC6
#define	ATA_CMD_ID		0xEC	/* identify ATA device */

//helper functions(reads)
#define ata_read_error(address)		(inb((address)+ERR_REG))
#define ata_read_status(address)	(inb((address)+STATUS_REG))
#define ata_read_sector_count(address)	(inb((address)+SECT_CNT_REG))
#define ata_read_feature(address)	(inb((address)+FEATURE_REG))
#define ata_read_alt_status(address)	(inb((address)+ALT_ST_REG))

//helper functions(writes)
#define ata_write_feature(address,data)		(outb((address)+FEATURE_REG,(data)))
#define ata_write_drv_ctrl(address,data)	(outb((address)+DEV_CTRL_REG,(data)))
#define ata_write_cmd(address,data)		(outb((address)+CMD_REG,(data)))
#define ata_write_head(address,data)		(outb((address)+DRV_HD_REG,(data)))
#define	ata_write_lba(address,lba)		(outb((address)+LBA_LOW_REG,(lba&0x0000ff))) (outportb((address)+LBA_MID_REG,(lba&0x00ff00)>>8)) (outportb((address)+LBA_HI_REG,(lba&0xff0000)>>16)) 

#define NUM_IO_SPANS	2
#define	read_le16(X)	*(word *)(X)
#define	read_be16(X)	bswap16(*(word *)(X))
#define read_le32(X)	*(dword *)(X)

// devtype defines
#define PATA	0
#define PATAPI	1
#define SATA	2
#define	SATAPI	3

// Primary or Secondary channel's register values
typedef struct channel  
{
	word base_reg; // base reg for the challel 
	word ctrl_reg; // ctrl reg for the channel
	word bmide;    // bus master IDE same for both channel but we will put
                                 // it in every instance  
	dword   intr;	 // Interrupt assigned to this channel
	byte nIEN;  // no interrupt 
}  channel;

extern channel ctrl_channel[2];

// the structure to represent one ide/PATA drive
typedef struct IDEdrive
{
	channel 	*chan;		// structure to represent the channel of this IDE device
	word	devnum:1;	// master = 0 slave = 1
	word	devtyp:2;	// ATA or ATAPI,SATA,SATAPI
	word	lba:2;		// no LBA, LBA-28,LBA-48
	word	use_lba:1;	// are we going to use LBA(1) or CHS(0)
	word	dma:1;		// DMA capable???
	word	use_dma:1;	// use dma(1) don't use(0)
	word	multimode:1;	// has multi mode 
	word	use_multimode:1;// use multimode ??
	word	mult_count;
	word  heads;
	word  sectors;
	word  cylinders;
	dword	totalsectors;
	byte	model_name[41];	// 40 char name nullterminated
	byte   serial[20];
	byte	firmware[8];
}  IDEdrive;

// partition entry in the drive
typedef struct part_entry
{
	dword boot_flag:8;
	dword beg_head:8;
	dword beg_sect:8;
	dword beg_cyl:8;
	dword part_type:8;
	dword end_head:8;
	dword end_sect:8;
	dword end_cyl:8;
	dword  beg_lba;
	dword  tot_sect;
}  part_entry_t;

typedef struct IdentifyData
{
    word type;
    word reserved1[9];
    byte  serial[20];
    word reserved2[3];
    byte  firmware[8];
    byte  model[40];
    word maxTransfer; // max num of sects to be xfered in read/write multiple
    word trustedFeatures;
    word capabilities[2];
    word reserved3[8];
    dword sectors28;
    word reserved4[18];
    word majorRevision;
    word minorRevision;
    word supported[6]; // command set supported
    word reserved5[12];
    unsigned long long sectors48;
    word reserved6[2];
    word sectorSize;
}  identify_data;

typedef struct ata_ident
{
	word discard1[10]; // 10 shorts -> 10 
	byte  slnum[20];   // 10 shorts->20
	word discard2[3]; // 3 shorts ->23
	byte  fw_rev[8];   // 4 shorts ->27
	byte  model[40];   // 20 shorts->47
	word discard3[2]; //  2 shorts->49
	dword   capability;  //  2       ->51 DMA 8th bit lba 9th bit
	word doscard4[9]; //  9       ->60
	dword   lba28maxsects;// 2       ->62
	word discard5[2];  // 2       ->64
	word pio_modes_supported; //1 ->65  // 0-7 bits to be checked
	word discard6[15]; // 15      ->80 
	word major_ata_ver;//  1      ->81
	word minor_ata_ver;//  1      ->82
	dword   cmdset_supported;//2     ->84
	word discard7[4];   //   4     ->88
	word ultraDMAfetures;// 1     ->89
	word discard8[11];   // 11     ->100
	unsigned long long lba48maxsects;//4    ->104
	word discard9[23];//  23      ->127
	word discard10[129];//129	->256  
}  ata_ident;

bool pio_wait_ready(word,bool);
bool ata_identify(IDEdrive *drv);

class disk
{
	private:
		IDEdrive *physical;
		bool has_valid_partition_tbl;
		
		//byte sect_buf[512]={0};
	public:
		disk()
		{
			physical=new IDEdrive();
			has_valid_partition_tbl=false;
		};

		~disk()
		{
			delete physical;
		};

		part_entry_t part_table[4]; //drive has 4 primary partitions
		void populate_ide_disk(channel *chan, word unit)
		{
			physical->chan=chan;
			physical->devnum=unit;
		};

		bool wait_ready(word port)
		{
			return (pio_wait_ready(port,true));
		};
		
		//int wait_interrupt(word int_mask,dword timeout);
		bool identify()
		{
			return (ata_identify(physical));
		}
		
		void read_sector(dword blk,byte *read_buf){};
		void write_sector(dword blk,byte *buf){};
		void part_ent_info(int i){};
		
		//static void disk_handler(IDT::regs *r);
		void disk_info(){};
		void populate_partitions(){};
		//void disk_handler(IDT::regs *r){};
		bool is_partitioned(){ return has_valid_partition_tbl;};
		bool is_valid_part_entry(int partn){ if(part_table[partn].tot_sect>0)
							return true;
						     else
							return false;
						    };
		void read_first_sector_of_partition(int partn,byte *buf)
		{
			//please do a checks.			
			read_sector(part_table[partn].beg_lba,buf);
		};
};

extern disk *disks[4];

void init_disks();

//int ide_select(ide_t *ide);
bool detect_cntrlr(word port);
void detect_ide();
void read_sector(word port,dword blk,char *read_buf);

//void ide_handler(regs *r);
void dump(void *data_p, unsigned count);
void hex_dump (const byte *data, int len);

//void ide_read_handler(regs *r);
void display_ide();

#endif
