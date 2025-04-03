#include "System.h"
#include "pci.h"
#include "ide.h"
#include "devata.h"
#include "txtvideo.h"
#include "MemoryManager.h"

DEVATA::DEVATA(pci_bus *pci_data,TEXTAREA *text_zone,TIMER *timer)
{
	textarea=text_zone;
	tmr=timer;
	memset(my_drives,NULL,4*sizeof(dword));
	memset(channels,NULL,2*sizeof(chan));
	slots=NULL;
	pci=pci_data;
	init_disks();

	textarea=text_zone;

	//if(n<0 || n>4)
		part_num=0;
	//else
		//part_num = n;

	strcpy(name,"hd");
	if(slots->ps)
	{
		if(slots->ms)
			strcat(name,"d");
		else
			strcat(name,"c");
	}
	else
	{
		if(slots->ms)
			strcat(name,"b");
		else
			strcat(name,"a");
	}
	
	switch(part_num)
	{
		case 0:	strcat(name,"-1");
			break;
		case 1: strcat(name,"-2");
			break;
		case 2: strcat(name,"-3");
			break;
		case 3: strcat(name,"-4");
	}

	guess_fs_type();
}

DEVATA::~DEVATA(){}

//==========================|helper functions|==============================
// depending on the second parameter sz
// reads a byte, word or dword from 
// port
dword pio_inport( word port , size_t sz)
{
	switch(sz)
	{
		case 1: return inb(port);
		case 2: return inw(port);
		case 4: return ind(port);
		default: return 0;
	}
}

// depending on the 3rd parameter sz
// sends a val byte/word/dword to
// port
void pio_outport(word port, dword val, size_t sz)
{
	switch(sz)
	{
		case 1: outb(port,(byte)val); break;
		case 2: outw(port,(word)val); break;
		case 4: outd(port,(dword)val); break;
	}
}

// reads single byte from port and returns it
byte pio_inbyte(word port)
{
	return (byte)pio_inport(port,1);
}

// reads a word 2 byts or short int
word pio_inword(word port)
{
	return (word)pio_inport(port,2);
}

// reads a dword(4-byte) from port and returns it
dword pio_indword(word port)
{
	return (dword)pio_inport(port,4);
}

// sends a byte value val to port
void pio_outbyte(word port,byte val)
{
	pio_outport(port,val,1);
}

// sends a short value val to port
void pio_outword(word port,word val)
{
	pio_outport(port,val,2);
}

// sends a int(4-byte) val to port
void pio_outdword(word port,dword val)
{
	pio_outport(port,val,4);
}

// reads count number of bytes from port to buffer
void pio_rep_inb(word port, byte *buffer, dword count)
{
	insb(port,buffer,count);
}

// reads count number of shorts from port to buffer
void pio_rep_inw(word port, word *buffer, dword count)
{
	insw(port,buffer,count);
}

// reads count number of ints from port to buffer
void pio_rep_indw(word port, dword *buffer, dword count)
{
	insd(port,buffer,count);
}

void pio_rep_outb(word port, byte *buffer, dword count)
{
	outsb(port, buffer, count);
}

void pio_rep_outw(word port, word *buffer, dword count)
{
	outsw(port, buffer, count);
}

void pio_rep_outdw(word port, dword *buffer, dword count)
{
	outsd(port, buffer, count);
}

byte pio_get_status(word port)
{
	return pio_inbyte(port+STATUS_REG);
}

byte pio_get_astatus(word port)
{
	return pio_inbyte(port+ALT_ST_REG);
}

bool DEVATA::pio_wait_busy(word port)
{
	// should have a delay here !!! guess it will work
	for(int i=0;i<4;i++)
		tmr->timer_wait(10); 
	return(pio_get_status(port)&STA_BSY);  // true if busy else false
}

bool DEVATA::pio_wait_busy_astat(word port)
{
	for(int i=0;i<4;i++)
		tmr->timer_wait(10); 
	return(pio_get_astatus(port)&STA_BSY);
}

bool is_device_ready(slot *s)
{
	return(pio_inbyte(s->chanl->base_reg + ALT_ST_REG)&STA_DRDY);
}

bool is_device_busy(slot *s)
{
	return(pio_inbyte(s->chanl->base_reg + ALT_ST_REG)&STA_BSY);
}

// stop sending interrupts
// this should be called after selecting a drive 
void stop_ata_intr(word ctrl_port)
{
	pio_outbyte(ctrl_port,ATA_CTL_nIEN);
}

// reset the drives on this controller
// call this if some drive behave insane
// or in the begining of driver
bool DEVATA::reset_controller(word port)
{
	char line[200];
	pio_outbyte(port + DEV_CTRL_REG,ATA_CTL_SRST|ATA_CTL_nIEN);
	tmr->timer_wait(2);
	sprintf(line,"Stopping interrupt for channel 0x%04X",port);
	textarea->AddLine(line);
	pio_outbyte(port + DEV_CTRL_REG,ATA_CTL_nIEN);
	tmr->timer_wait(2);
	dword timeout=300000;
	while(pio_wait_busy(port))
	{
		timeout--;
		tmr->timer_wait(1);
	};
	if(!timeout)
		textarea->AddLine("Device is in busy state for a long time even after reset");
	word err=pio_inbyte(port+ERR_REG);
	if(err)
	{
		sprintf(line,"Warning Error flag is %d | ALT_ST_REG=%d",err,(word)pio_inbyte(port+ALT_ST_REG));
		textarea->AddLine(line);
	}
	if(timeout)
		return true;
	return false;
}

#define IDENTIFY_TEXT_SWAP(field,size) {byte tmp; for (int i = 0; i < (size); i+=2) { tmp = (field)[i]; (field)[i]   = (field)[i+1]; (field)[i+1] = tmp; } }

//===========================================================================
// this function will check a master drive in a channel
// return true if found
// return false if not
// takes the base port for the channel
bool DEVATA::detect_master(word port)
{
	int tmp;
	outb(port + DRV_HD_REG, 0xA0);	// Set drive
	pio_wait_busy(port);	
	tmp = inb(port+STATUS_REG);	// Read status
	if (tmp & STA_DRDY)
		return true;
	else
		return false;
}

bool DEVATA::detect_slave(word port)
{
	int tmp;
	outb(port + DRV_HD_REG, 0xB0);	// Set drive
	pio_wait_busy(port);	
	tmp = inb(port+STATUS_REG);	// Read status
	if (tmp & STA_DRDY)
		return true;
	else
		return false;
}

bool DEVATA::search_disks(pci_bus *pci)
{
	char line[200];
	dword cmdbase_pri=ATA_BASE_PRI,cmdbase_sec=ATA_BASE_SEC;
	dword ctrlbase_pri,ctrlbase_sec,bmidebase,bmidebase_pri,bmidebase_sec;
	pci_bus *pb=pci;
	pci_dev *dev=NULL;
	
	textarea->AddLine("Initializing IDE harddisks");
	textarea->AddLine("Checking PCI bus for IDE");
	dev=pb->get_dev((byte) 0x01,(byte)0x01);
	if(dev && (dev->next!=NULL))
	{
		textarea->AddLine("WARNING: No support for more than one one IDE card");
		textarea->AddLine("Default is the first card detected");
		sprintf(line,"On this machine dev-bus=%d  dev=%d  func=%d",dev->bus,dev->dev,dev->func);
		textarea->AddLine(line);
	}
	
	if(dev!=NULL)
	{
		sprintf(line,"vendorID=%04X deviceID=%04X bus=%d dev=%d func=%d IRQ=%d",dev->common->vendor_id,dev->common->device_id,dev->bus,dev->dev,dev->func,(dword)dev->irq);
		textarea->AddLine(line);
		cmdbase_pri=pci->get_bar(dev,0);
		ctrlbase_pri=pci->get_bar(dev,1);
		cmdbase_sec=pci->get_bar(dev,2);
		ctrlbase_sec=pci->get_bar(dev,3);
		bmidebase=pci->get_bar(dev,4);
		bmidebase &=0xfffe;
		if(cmdbase_pri==0xffff || ctrlbase_pri==0xffff || bmidebase==0xffff)
		{
			textarea->AddLine("Error in PCI config for primary channel");
		}
		else
		{
			cmdbase_pri &= 0xfffe;
			ctrlbase_pri &= 0xfffe;
			if(cmdbase_pri==0)
			{
				textarea->AddLine("Device doesn't provide specific reg for primary ...defaulting");
				cmdbase_pri=ATA_BASE_PRI;
				ctrlbase_pri=cmdbase_pri + DEV_CTRL_REG;
			}
			else
				ctrlbase_pri -=4;
			bmidebase_pri=bmidebase;
			channels[0].base_reg = cmdbase_pri;
			channels[0].ctrl_reg = ctrlbase_pri;
			channels[0].bmide  =   bmidebase_pri;
			channels[0].nIEN =     0;
		}
		if(cmdbase_sec==0xffff || ctrlbase_sec==0xffff || bmidebase==0xffff)
		{
			textarea->AddLine("Error in PCI config for secondary channel");
		}
		else
		{
			cmdbase_sec &= 0xfffe;
			ctrlbase_sec &= 0xfffe;
			if(cmdbase_sec==0)
			{
				textarea->AddLine("Device doesn't provide specific reg for secondary... defaulting");
				cmdbase_sec=ATA_BASE_SEC;
				ctrlbase_sec=cmdbase_sec + DEV_CTRL_REG;
			}
			else
				ctrlbase_sec -=4;
			bmidebase_sec =bmidebase+8;
			channels[1].base_reg = cmdbase_sec;
			channels[1].ctrl_reg = ctrlbase_sec;
			channels[1].bmide  =   bmidebase_sec;
			channels[1].nIEN =     0;
		}
	}
	else
	{
		textarea->AddLine("No PCI IDE found");
		return false;
/*
		cout<<"Using Default IDE ports\n";
		channels[0].base_reg=ATA_BASE_PRI;
		channels[0].ctrl_reg=0x3F6;
		channels[0].bmide=-1;
		channels[0].nIEN=0;
		channels[1].base_reg=ATA_BASE_SEC;
		channels[1].ctrl_reg=0x376;
		channels[1].bmide=0;
		channels[1].nIEN=0;*/
	}
	textarea->AddLine("Details of channels found");
	for(int k=0;k<2;k++)
	{
		sprintf(line,"chanel=%d base-reg=0x%04X ctrl-reg=0x%04X bmide=0x%04X nIEN=0x%04X",k,channels[k].base_reg,channels[k].ctrl_reg,channels[k].bmide,channels[k].nIEN);
		textarea->AddLine(line);
	}
	textarea->AddLine("Reseting controllers");
	if(!reset_controller(channels[0].base_reg))
		textarea->AddLine("can't reset channel 0");
	if(!reset_controller(channels[1].base_reg))
		textarea->AddLine("can't reset channel 1");
	
	for(int c=0,s=0;c<2;c++)
	{
		if(detect_master(channels[c].base_reg))
			my_drives[s]=1;
		s++;
		if(detect_slave(channels[c].base_reg))
			my_drives[s]=1;
		s++;
	}

	slot *temp=NULL,*cur_slot=NULL;

	for(int k=0;k<4;k++)
	{	// for every entry in my_drives we will create a linklist of drives
		//textarea->AddLine("Found drives");
		if(my_drives[k]==1)
		{
		 	//cout<<k<<" ";
			temp = new slot;
			if(!temp)
			{
				//cout<<"Insufficient Memory for slot information\n";
				textarea->AddLine("Insufficient Memory for slot information");
				while(1);
			}
			memset(temp,'\0',sizeof(slot));
			temp->chanl = new chan;
			memset(temp->chanl,'\0',sizeof(chan));
			
			temp->exists = 1;
			temp->drv_number = k;
			temp->next=NULL;
			if(k%2)
				// slave device
				temp->ms = 1;
			else
				// master device
				temp->ms = 0;
			if(k<2)
			{
				// primary channel
				temp->chanl->base_reg = channels[0].base_reg;
				temp->chanl->ctrl_reg = channels[0].ctrl_reg;
				temp->chanl->bmide = channels[0].bmide;
				temp->chanl->nIEN  = channels[0].nIEN;
				temp->ps = 0;
			}
			else
			{
				// secondary channel
				temp->chanl->base_reg = channels[1].base_reg;
				temp->chanl->ctrl_reg = channels[1].ctrl_reg;
				temp->chanl->bmide    = channels[1].bmide;
				temp->chanl->nIEN     = channels[1].nIEN;
				temp->ps = 1;
			}
			if(slots==NULL)
			{
				slots = temp;
				cur_slot=temp;
			}
			else
			{
				cur_slot->next = temp;
				cur_slot=cur_slot->next;
			}
		}	
	}
	//cout<<"\n";		
	//cout.flags(dec);
	return true;		
}

void select_device(slot *s)
{
	word port;
	byte val=0;
	port = s->chanl->base_reg + DRV_HD_REG;
	if(s->ms)
		val = 0xB0 ;
	else
		val = 0xA0 ;
	pio_outbyte(port,val);
}

void DEVATA::browse_slots()
{
	slot *temp;
	temp = slots;
	char line[200];
	
	byte id_cmd=0x00;
	ata_ident *dat = new ata_ident;
	memset(dat,'\0',sizeof(ata_ident));
	/*while(is_device_busy(temp))
	{
		cout<<"device is busy!!! sleeping\n";
		TIMER::timer_wait(300);
	};*/
	//cout.flags(hex|showbase);
	while(temp)
	{	
		sprintf(line,"Selecting: Channel=%s Slot=%s Drive=%d",temp->ps ? "Secondary ":"Primary ",temp->ms ? "Slave ":"Master ",temp->drv_number);
		textarea->AddLine(line);
		select_device(temp);
		if(is_device_ready(temp))
		{
			textarea->AddLine("device is ready");
		}
		else 
		{
			textarea->AddLine("device is not ready");
			continue;
		}
		if(is_device_busy(temp))
			textarea->AddLine("device is Busy");
		pio_outbyte(temp->chanl->base_reg + CMD_REG, ATA_CMD_ID); 

		// Now if it is pata drive it will put an error flag in flags register
		if(pio_inbyte(temp->chanl->base_reg + ERR_REG) & STA_ERR)
		{
			temp->devtype = PATA;
			id_cmd=ATA_CMD_ID; // ATA Identify
		}
		else
		{
			byte temp1=0;
			byte temp2=0;
			
			temp1 = inb(temp->chanl->base_reg + LBA_MID_REG);
			temp2 = inb(temp->chanl->base_reg + LBA_HI_REG);

			sprintf(line,"temp1 = 0x%04X  |  temp2 = 0x%04X",temp1,temp2);
			textarea->AddLine(line);
			
			if(temp1 == 0x14 && temp2 == 0xEB)
			{
				temp->devtype = PATAPI;
				id_cmd=ATA_CMD_PID; // ATAPI identify
			}
			if(temp1 == 0x69 && temp2 == 0x96)
			{
				temp->devtype = SATAPI;
				id_cmd=ATA_CMD_PID; // we don't know how to handle it
			}
			if(temp1 == 0 && temp2 == 0 )
			{
				temp->devtype = PATA;
				id_cmd=ATA_CMD_ID; // ATA Identify
			}
			if(temp1==0x3c && temp2 == 0xc3 )
			{
				temp->devtype = SATA;
				id_cmd=0; // we don't know how to handle it
			}
		}
		sprintf(line,"ID-CMD = %04X",id_cmd);
		textarea->AddLine(line);
		switch(id_cmd)
		{
			case ATA_CMD_ID : //we already sent it so we only will read if STA_DRQ status is present
						//pio_outbyte(temp->chanl->base_reg + CMD_REG, ATA_CMD_ID); 
						if(pio_get_status(temp->chanl->base_reg) & STA_DRQ)
						{
							pio_rep_inw(temp->chanl->base_reg,(word *)dat,256);
							IDENTIFY_TEXT_SWAP(dat->model,40);
							temp->heads = dat->discard1[6];
							temp->sectors = dat->discard1[3];
							temp->cylinders = dat->discard1[1];
							temp->sectors28= dat->lba28maxsects;
							temp->sectors48= dat->lba48maxsects;
							if((dat->capability & 1<<8) == 1<<8)
								temp->dma=1;
							if((dat->capability & 1<<9) == 1<<9)
								temp->lba=1;
							sprintf(line,"model = %04X",dat->model);
							textarea->AddLine(line);
							// here read the MBR then extract the partition table
							mbr *tmbr = new mbr;
							slots=temp;
							if(!ata_r_sector(0,(word *)tmbr))
								textarea->AddLine("Error reading MBR");
							else
							{
								//for the timebeing we are only intersted in partition table
								// copy it
								memcpy(temp->partition_table,tmbr->partitions,4*sizeof(partition)); 
							}
							free(tmbr);
						}
						break;
			case ATA_CMD_PID :
						pio_outbyte(temp->chanl->base_reg + CMD_REG, ATA_CMD_PID);
							pio_rep_inw(temp->chanl->base_reg,(word*)dat,256);
							IDENTIFY_TEXT_SWAP(dat->model,40);
							sprintf(line,"model = %04X",dat->model);
							textarea->AddLine(line);							
						break;
		}
		temp=temp->next;
	}
}

void DEVATA::display_slot_info()
{
	slot *temp;
	temp = slots;
	char line[200];

	textarea->AddLine("[Displaying slot infornations]");
	while(temp)
	{
		sprintf(line,"%s %s %s %s %s heads:%d sect:%d cyls:%d lba28sects=%d lba48sects=%d",(temp->ps ? "sec ":"pri "),(temp->ms ? "slv ":"mst "),(temp->devtype ? "SATA(PI)/PATAPI ":"PATA "),(temp->lba ? " LBA ":"CHS "),(temp->dma ? "DMA ":"no DMA "),temp->heads,temp->sectors,temp->cylinders,temp->sectors28,temp->sectors48);
		textarea->AddLine(line);
		if(temp->devtype)
		goto ml;
		if(temp->partition_table)
		{
			// show the partition s with infos
			for(int p=0;p<4;p++)
			{
				//cout<<(int)p+1<<" ";
				display_partition_info(&temp->partition_table[p]);
			}
		}
	ml:
		temp=temp->next;
	}
}

slot *DEVATA::get_device(word type)
{
	slot *temp=NULL,*dev1=NULL,*dev=NULL;
	char line[200];
	temp = slots;
	if(!temp)
		return NULL;
	while(temp)
	{
		if(temp->devtype == type)
		{
			if(dev1==NULL)
			{
				dev=temp;
				dev->next=NULL;
				dev1=dev;
			}
			else
				dev->next=temp;	
			dev=dev->next;
		}
		temp = temp->next;
	}
	//cout<<"nodevice of type ="<<type<<" found\n";
	sprintf(line,"nodevice of type [%d] found",type);
	textarea->AddLine(line);
	return dev1;
}

int wait_ready(word port,dword timeout)
{
	byte stat;
	outb(port+ERR_REG,0);
	while(timeout)
	{
		stat=ata_read_status(port);
		if(!stat) return 1;
		if((stat & (STA_DRDY|STA_BSY))==STA_DRDY) return 1;
		timeout--;
	}
	return -1;
}

// generic function to read or write a sector
// drv is the ide drive , block is LBA , direction is read =0 write =1
dword DEVATA::ata_rw_sector(slot *drv,dword block,word *buf,byte direction) 
{
	byte sc, cl, ch, hd, cmd;
	dword timeout=300000;
	word iobase = drv->chanl->base_reg;
	//iobase = drv->chanl->base_reg;
	if(drv->sectors28<block)
		return 0;
	/* put exclusion thigs here */
	//select_device(drv);
	/*{
		cout<<"select drive failed \n";
		// uninitialize exclusion here
		return 0;	
	}*/
	//stop_ata_intr(drv->chanl->ctrl_reg);
	if (drv->lba) 
	{
		sc = block & 0xff;
		cl = (block >> 8) & 0xff;
		ch = (block >> 16) & 0xff;
		hd = (block >> 24) & 0x0f;
		//if(drv->ms)
		//	hd |=(1<<4);
		//if(drv->ps)
		//	hd |=0xf0;
	} 
	else 
	{
        /* See http://en.wikipedia.org/wiki/CHS_conversion */
	        int cyl = block / (drv->heads * drv->sectors);
	        int tmp = block % (drv->heads * drv->sectors);
	        sc = tmp % drv->sectors + 1;
	        cl = cyl & 0xff;
	        ch = (cyl >> 8) & 0xff;
	        hd = tmp / drv->sectors;
	}
	if(direction == 0)
	{
		cmd= ATA_CMD_READ;
	}
	else
	{
		cmd= ATA_CMD_WRITE;	
	}
	pio_outbyte(iobase + FEATURE_REG,0); // ????
	pio_outbyte(iobase + SECT_CNT_REG, 1); //we want only one sector
	pio_outbyte(iobase + LBA_LOW_REG, sc);
	pio_outbyte(iobase + LBA_MID_REG, cl);
	pio_outbyte(iobase + LBA_HI_REG, ch);
	pio_outbyte(iobase + DRV_HD_REG, 0xE0|hd|drv->ms<<4);//(drv->ms<<4)|0xE0|hd should be passed 
						//according to http://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO
						// but it never works
	pio_outbyte(iobase + CMD_REG, cmd);
	/* The host shall wait at least 400 ns before reading the Status register.
	See PIO data in/out protocol in ATA/ATAPI-4 spec. */
	tmr->timer_wait(1);
	//cout<<" Reading hd= "<<(int)hd<<" sect= "<<(int)sc<<" cyl= "<<(int)cl<<"\n";
	//while(pio_inbyte(iobase+DATA_REG)==0x08);
/*	while (timeout)	
	{
		// wait for busy flag to clear
		if(!pio_inbyte(iobase + STATUS_REG)& STA_BSY)
			break;
		timeout--;
		tmr->sleep(1);
	}
	if(timeout==0)
	{
		// put unlock for mutex here
		cout<<"Time out but device never came back from busy state\n";	
		return 0;
	}*/
	/* Did the device report an error? */
	if (pio_inbyte(iobase + ALT_ST_REG) & STA_ERR) 
	{
		//put unlock mutex 
		//cout<<"Error in operation\n";
		textarea->AddLine("Error in operation");
		return 0;
	}
	// we are ok till now
	// as we habe stopped the interrupt we should poll data
	/*timeout = 300000;
	for(; timeout>0; timeout--)
	{
		if(pio_inbyte(iobase + STATUS_REG) & STA_DRQ)
			break;
	}
	if(timeout==0)
	{
		// put unlock for mutex here
		cout<<"time out waiting for data request\n";	
		return 0;
	}*/
	// well our request is successfull
	// now read it to the output buffer
	while(!(pio_inbyte(iobase + ALT_ST_REG) & STA_DRQ));
	if(direction == 0)
	{
		//cmd= ATA_CMD_READ;
		pio_rep_inw(iobase + DATA_REG, buf, 256);
		while(is_device_busy(drv))
			;
	}
	else
	{
		//cmd= ATA_CMD_WRITE;
		pio_rep_outw(iobase + DATA_REG, buf, 256);	
	}
	return 1;
}

dword DEVATA::ata_r_sector(dword block,word *buf)
{
	return ata_rw_sector(slots, block, buf,0);
}

dword DEVATA::ata_w_sector(dword block,word *buf)
{
	return ata_rw_sector(slots, block, buf,1);
}

void DEVATA::display_partition_info(partition *p)
{
	char line[200];

	if(p)
	{
		word shd=0,ssc=0,scy=0,ehd=0,esc=0,ecy=0, cyl_hi=0;
		shd = p->starting_head;
		ssc = p->starting_sec_cyl & 0x003F;
		scy = p->starting_sec_cyl;
		scy >>=6;   // 10 bits remain  the lower 2 bits are actually spill over of higher 2 bits of 10 bit
		cyl_hi= scy & 0x0003; // higher 2bits of cylinder
		scy >>=2;        // only lower 8 bits
		scy = scy | (cyl_hi<<8) ; // covert the cylinder back to normal by putting higher 2bits
		ehd = p->ending_head;
		esc = p->ending_sec_cyl & 0x003F;
		ecy = p->ending_sec_cyl;
		ecy >>=6;
		cyl_hi = scy & 0x0003;
		ecy >>=2;
		ecy = ecy | (cyl_hi<<8);
		sprintf(line,"   start-LBA=0x%08X  total-sects=0x%08X  Sys-ID=%d",p->start_lba,p->total_sectors,p->system_id);
		textarea->AddLine(line);
	}
}

/*
void DEVATA::init_sysdrives()
{
	slot *temp=NULL;
	temp = slots;
	char line[200];
	
	textarea->AddLine("Initializing System drives");
	int i=0;
	while(temp)
	{
		if(temp->devtype) // PATA 0
			goto ml;
		else
		{
			textarea->AddLine("found a PATA drive");
			if(temp->partition_table)
			{
				for(int p=0;p<4;p++)
				{
					if(temp->partition_table[p].total_sectors>0)
					{
						drive *d = new drive(temp, p,textarea);
						sysdrives[i] = d;
						i++;
					}
				}
			}
		}
	ml:
		temp=temp->next;
	}
	sprintf(line,"Total drives found = %d",i);
	textarea->AddLine(line);
}

void DEVATA::display_sysdrive_info()
{
	char ans[5];
	for(int i=0;i<16;i++)
	{
		if(sysdrives[i]!=NULL)
		{
			sysdrives[i]->display_info();
			//sysdrives[i]->fs->dump_fat_info();
			//sysdrives[i]->fs->display_root_dir();
		}
	}
}*/

void DEVATA::init_disks()
{
	if(search_disks(pci))
	{
		browse_slots();
		display_slot_info();
		//init_sysdrives();
		//display_sysdrive_info();
	}
	else
		textarea->AddLine("NO IDE disks!!!");
	/*char ans[5];*/
	/*if(slot *temp = get_device(0))
	{
		word *sector = new word[256];
		dword stlba = temp->partition_table[0].start_lba;
		cout<<"Reading "<<stlba<<" \n";
		if(ata_r_sector(temp,stlba,sector))
		hex_dump((byte*)sector,512);
	}*/	
}

char *DEVATA::get_name()
{
	return (name);
}

void DEVATA::set_partition(word p)
{
	part_num = p;
}

word DEVATA::get_partition()
{
	return (part_num); 
}

char *DEVATA::fs_type_to_string(word fs_type)
{
	switch(fs_type)
	{
		case 0x01: //cout<<"FAT12\n";
				return "FAT12";
		case 0x04: 
		case 0x05:
		case 0x06:
		case 0x0e: //cout<<"FAT16\n";
				return "FAT16";
		case 0x07: //cout<<"NTFS\n";				
				return "NTFS";
		case 0x0b: 
		case 0x0c: //cout<<"FAT32\n";
				return "FAT32";
		case 0x83: //cout<<"EXT2\n";
				return "EXT2";
		default: //cout<<"Unknown File System\n";
				return "Unknown FS";
	}
}

void DEVATA::display_info()
{
	char line[100];
	char *fstype;

	sprintf(line,"Drive: %s partition",name);
	textarea->AddLine(line);
	//suspendat momentan
	//display_partition_info(&drive_slot->partition_table[part_num]);
	fstype=fs_type_to_string(fs_type);
	sprintf(line,"Filesystem type: %s",fstype);
	textarea->AddLine(line);
}

void DEVATA::set_fs_type(byte fst)
{
	fs_type=fst;
}

void DEVATA::guess_fs_type()
{
	char line[100];

	fs_type = slots->partition_table[part_num].system_id;
	sprintf(line,"Guessed File System ID = %d  name=[%s]",fs_type,fs_type_to_string(fs_type));
	guessed = true;
}

dword DEVATA::read(dword block, byte *buffer)
{
	dword st,en;
	st= slots->partition_table[part_num].start_lba;
	en = st + slots->partition_table[part_num].total_sectors;
	block += st; 
	if((block < st) || (block > en))
	{
		textarea->AddLine("Trying to access out side the boundary");
		return 0;	
	}
	return ata_r_sector(block,(word *)buffer);
}

