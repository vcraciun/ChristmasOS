#ifndef __PCI_H__
#define __PCI_H__

#include "types.h"
#include "TextArea.h"

//! PCI command register (offset).
#define PCI_COMMAND			0x04
//! PCI status register (offset).
#define PCI_STATUS			0x06
//! using the ports).
#define PCI_COMMAND_IO			0x01
//! The device is memory-based (a.k.a. can perform I/O operations
//! by a memory-mapped buffer)
#define PCI_COMMAND_MEMORY		0x02
//! The PCI cache line size register (offset).
#define PCI_CACHE_LINE_SIZE		0x0C
//! Enable bus master (a.k.a. 32-bit DMA).
#define PCI_COMMAND_MASTER		0x04
//! PCI latency timer register (offset).
#define PCI_LATENCY_TIMER		0x0D
#define PCI_BASE_REG			0xCFC
#define PCI_DATA_REG			0xCF8
//! PCI interrupt line register (offset).
#define PCI_INTERRUPT_LINE		0x3C
//! PCI interrupt pin register (offset).
#define PCI_INTERRUPT_PIN		0x3D
//! This is a normal PCI device.
#define PCI_HEADER_TYPE_NORMAL		0
//! This is a bridge PCI device.
#define PCI_HEADER_TYPE_BRIDGE		1
//! This is a card-bus PCI device.
#define PCI_HEADER_TYPE_CARDBUS		2

#define L1_CACHE_BYTES			(1 << 4) //for 386

typedef struct PCI_common
{
	word vendor_id;
	word device_id;
	word cmd_reg;
	word status_reg;
	byte revision_id;
	byte Prog_if;
	byte subclass;
	byte classcode;
	byte cacheline_size;
	byte latency;
	byte header_type;
	byte BIST;		//self test
} PCI_common;

struct non_bridge
{
	dword base_address0 ;
	dword base_address1 ;
	dword base_address2 ;
	dword base_address3 ;
	dword base_address4 ;
	dword base_address5 ;
	dword CardBus_CIS ;
	word  subsystem_vendorID ;
	word  subsystem_deviceID ;
	dword expansion_ROM ;
	byte  cap_ptr ;
	byte  reserved1[3] ;
	dword reserved2[1] ;
	byte  interrupt_line ;
	byte  interrupt_pin ;
	byte  min_grant ;
	byte  max_latency ;
	dword device_specific[48] ;
};

struct bridge
{
	dword base_address0 ;
	dword base_address1 ;
	byte  primary_bus ;
	byte  secondary_bus ;
	byte  subordinate_bus ;
	byte  secondary_latency ;
	byte  IO_base_low ;
	byte  IO_limit_low ;
	word  secondary_status ;
	word  memory_base_low ;
	word  memory_limit_low ;
	word  prefetch_base_low ;
	word  prefetch_limit_low ;
	dword prefetch_base_high ;
	dword prefetch_limit_high ;
	word  IO_base_high ;
	word  IO_limit_high ;
	dword reserved2[1] ;
	dword expansion_ROM ;
	byte  interrupt_line ;
	byte  interrupt_pin ;
	word  bridge_control ;
	dword device_specific[48] ;
};

struct cardbus
{
	dword ExCa_base ;
	byte  cap_ptr ;
	byte  reserved05 ;
	word  secondary_status ;
	byte  PCI_bus ;
	byte  CardBus_bus ;
	byte  subordinate_bus ;
	byte  latency_timer ;
	dword memory_base0 ;
	dword memory_limit0 ;
	dword memory_base1 ;
	dword memory_limit1 ;
	word  IObase_0low ;
	word  IObase_0high ;
	word  IOlimit_0low ;
	word  IOlimit_0high ;
	word  IObase_1low ;
	word  IObase_1high ;
	word  IOlimit_1low ;
	word  IOlimit_1high ;
	byte  interrupt_line ;
	byte  interrupt_pin ;
	word  bridge_control ;
	word  subsystem_vendorID ;
	word  subsystem_deviceID ;
	dword legacy_baseaddr ;
	dword cardbus_reserved[14] ;
	dword vendor_specific[32] ;
};

typedef struct confadd
{
    byte reg:8;
    byte func:3;
    byte dev:5;
    byte bus:8;
    byte rsvd:7;
    byte enable:1;
} confadd_t;

struct pci_dev
{
	dword bus,dev,func;
	PCI_common *common;
	byte irq;
	dword devi[60];
	pci_dev *next;
	pci_dev *prev;
};

typedef enum bar_type{
	BAR_TYPE_MEM=0,
	BAR_TYPE_IO
}bar_type_t;

class pci_bus
{
	private:
		pci_dev *pci_list;
		pci_dev *end;
		dword num_dev;
		
		pci_bus();
		~pci_bus();

		static pci_bus* instance;
		
		//pci_bus(): pci_list(NULL),end(NULL),num_dev(0){};
	public:		
		static pci_bus* GetInstance();
		static void FreeInstance();

		void list_dev();
		void scan();
		pci_dev *get_dev(word vendor,word device);
		pci_dev *get_dev(byte classcode,byte subclass);
		pci_dev *get_dev_by_class(byte classcode);
		dword get_bar(pci_dev *dev,int bar_num);

	private:
		const char *vendor_to_string(word vend_id);
		const char *vendor_device_to_string(word vendor,word device);
		dword read_pci(int bus, int dev, int func, int reg, int uint8_ts);
		void write_pci(int bus, int dev, int func, int reg, dword v, int uint8_ts);
		byte pci_read_config_byte(int bus, int dev, int func, int reg);
		word pci_read_config_word(int bus, int dev, int func, int reg);
		dword pci_read_config_dword(int bus, int dev, int func, int reg);
		void pci_write_config_byte(int bus, int dev, int func, int reg, byte val);
		void pci_write_config_word(int bus, int dev, int func, int reg, word val);
		void pci_write_config_dword(int bus, int dev, int func, int reg, dword val);
		void pci_bus_scan();
		bar_type_t get_bar_type(pci_dev *dev, int bar_num);
		void pci_set_master(pci_dev *cfg);
		byte pci_set_pwr_state(pci_dev *dev, byte state);
		byte pci_get_capability(pci_dev *dev, byte *cap);
		byte pci_enable_device_io(pci_dev *dev);
		int pci_enable_device(pci_dev *dev);
		byte pci_read_irq(int bus,int dev,int func);
		dword is_pci_present();
		char *class_to_string(byte classcode,byte subclass);
};

#endif

