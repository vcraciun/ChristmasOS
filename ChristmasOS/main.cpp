#include "types.h"
#include "crt.h"
#include "Video.h"
#include "txtvideo.h"
#include "processor.h"
#include "MemoryManager.h"
#include "font8x8.h"
#include "font8x16.h"
#include "Disk.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "m_mouse.h"
#include "defines.h"
#include "System.h"
#include "TextArea.h"
#include "pci.h"
#include "devata.h"

#define DEBUG_Y_RES 768

extern dword default_pointer[290];
extern char *errmsgs[14];
extern TEXTAREA *pittext;
bool reset_request;
dword total_drives;
dword partitions;

void drawBackGround(TEXTAREA* textarea)
{
	char line[128];

	DISK::ChangeDisk(0);
	DISK::ReadSectors(DISK_CACHE, 0, 100);
	partitions = DISK::GetNumberOfPrimaryPartitions(DISK_CACHE);
	sprintf(line, "Total primary partitions on Drive0:  %d", partitions);
	textarea->AddLine(line);

	DIR g_sDirObject;
	FIL g_sFileObject;
	FILINFO g_sFileInfo;
	FATFS volume;
	dword result, ulDirCount, ulFileCount, ulTotalSize;
	char buffer[2000];
	dword read;
	FRESULT handle;

	FSFAT::f_mount(0, &volume);
	textarea->AddLine("Loading Background...");
	sprintf(line, "christmasos/data/backgrounds/nature.bmp", VIDEO::GetInstance()->GetWidth(), VIDEO::GetInstance()->GetHeight());
	handle = FSFAT::f_open(&g_sFileObject, line, FA_READ, textarea);
	sprintf(line, "Mounted volume: [%s]", g_sFileObject.fs->volname);
	textarea->AddLine(line);
	sprintf(line, "sectors per cluster = %d", g_sFileObject.fs->sects_clust);
	textarea->AddLine(line);
	byte* backgnd = (byte*)malloc(g_sFileObject.fsize);
	FSFAT::f_read(&g_sFileObject, backgnd, g_sFileObject.fsize, &read, textarea);
	textarea->AddLine((char*)backgnd);
	FSFAT::f_close(&g_sFileObject);
	sprintf(line, "Wallpaper: W = %d | H = %d | BPP = %d | raw_sz = %d | start = %d", *((dword*)(backgnd + 0x12)), *((dword*)(backgnd + 0x16)), *((word*)(backgnd + 0x1C)), *((dword*)(backgnd + 0x22)), *((dword*)(backgnd + 0x0A)));
	textarea->AddLine(line);
	VIDEO::GetInstance()->PutBMP(0, 0, backgnd);
}

int main()
{
	dword ptp[3];
	dword totalram;
	word videomode;
	dword vmem;
	char line[500];
	int i;

	reset_request=false;

	ClearScreen(1);
	ClearLine(7,0);
	SetAttr(7,0);
	GotoXY(1,0);
	printf("Loading ChristmasOS Boot-Manager! Please Wait...");
	ClearLine(7,24);
	SetAttr(7,0);
	GotoXY(1,24);
	printf("Querying System Hardware...");
	
	SetAttr(1,15);
	GotoXY(0,1);

	GetProcType(&ptp[0],&ptp[1],&ptp[2]);
	printf("Processor Type: [%c%c%c%c%c%c%c%c%c%c%c%c]\n",(char)(ptp[0]),(char)(ptp[0]>>8),(char)(ptp[0]>>16),(char)(ptp[0]>>24),(char)(ptp[1]),(char)(ptp[1]>>8),(char)(ptp[1]>>16),(char)(ptp[1]>>24),(char)(ptp[2]),(char)(ptp[2]>>8),(char)(ptp[2]>>16),(char)(ptp[2]>>24));
	totalram=GetRAMAmmount()/0x100000;
	printf("Total RAM Memory found: %d MB\n",totalram);

	//in initializarea modului grafic se face o trece scurta in REAL-MODE pentru a schimba modul video
	//schimbarea se face folosind noile tabele GDT, si nu cele initializate in MBR
	gdt_install();
	VIDEO *Screen=VIDEO::GetInstance(768);
	IDT* idt = IDT::GetInstance();

	Screen->set_font(FONT8x16_WIDTH,FONT8x16_HEIGHT,FONT8x16_NR_CHARS,font_data8x16);
	Screen->SetTextLimitLeft(0);
	vmem=GetVideoRAMAmmont(Screen->getlfb());

	KEYBOARD* keyboard = KEYBOARD::GetInstance();
	Mouse* mouse = Mouse::GetInstance();

	TEXTAREA *textarea=new TEXTAREA(10,50,Screen->GetWidth()-320,Screen->GetHeight()-320,500);
	textarea->SetColors(0,0xFFFFFF,0,0x80,0xFFFF00);
	textarea->set_font(FONT8x16_WIDTH,FONT8x16_HEIGHT,FONT8x16_NR_CHARS,font_data8x16);

	drawBackGround(textarea);

	Screen->plot_rectangle(0, 0, Screen->GetWidth(), Screen->GetFont().height + 1, true, 0x9090FF);
	Screen->GotoXY(0, 0);
	Screen->SetAttr(0, 0, false);
	Screen->gprintf("Runnning ChristmasOS");

	textarea->SetCaption("System Initialization");
	textarea->SetActive(true);
	textarea->Repaint();

	sprintf(line,"Total RAM Memory: %d MB",totalram);
	textarea->AddLine(line);
	sprintf(line,"Total Video Mem:  %d MB",vmem>>20);
	textarea->AddLine(line);
	sprintf(line,"Desktop Width:    %d",Screen->GetWidth());
	textarea->AddLine(line);
	sprintf(line,"Desktop Height:   %d",Screen->GetHeight());
	textarea->AddLine(line);
	sprintf(line,"Desktop BPP:      %d",Screen->GetBPP());
	textarea->AddLine(line);
	sprintf(line,"Video interface:  %c%c%c%c",*(char*)VESA_STRING,*(char*)(VESA_STRING+1),*(char*)(VESA_STRING+2),*(char*)(VESA_STRING+3));
	textarea->AddLine(line);
	sprintf(line,"Version:          %d.%d",*(char*)VESA_VER_HI,*(char*)(VESA_VER_LO+1));
	textarea->AddLine(line);
	sprintf(line,"Processor Type:   %c%c%c%c%c%c%c%c%c%c%c%c",(char)(ptp[0]),(char)(ptp[0]>>8),(char)(ptp[0]>>16),(char)(ptp[0]>>24),(char)(ptp[1]),(char)(ptp[1]>>8),(char)(ptp[1]>>16),(char)(ptp[1]>>24),(char)(ptp[2]),(char)(ptp[2]>>8),(char)(ptp[2]>>16),(char)(ptp[2]>>24));
	textarea->AddLine(line);
	sprintf(line,"Graphics Adaptor: %s",((*(WORD*)VESA_STRING_SEGM)<<4)|(*(WORD*)VESA_STRING_OFFS));
	textarea->AddLine(line);
	textarea->AddLine("Installing new Global Descriptor Table");
	textarea->AddLine("Installing Interrupt Descriptor Table");
	textarea->AddLine("Installing Interrupt System Services and Exceptions");	

	//textarea->AddLine("Loading FONTS...");
	//textarea->AddLine("Loading Theme...");
	//textarea->AddLine("Loading modules...");

	//textarea->AddLine("Installing KEYBOARD on IRQ1");
	keyboard->AddEventWatcherKey(textarea);

	textarea->AddLine("Installing TIMER on IRQ0");
	TIMER::GetInstance(50);
	TIMER::GetInstance()->EnableTimer();

	//textarea->AddLine("Scanning PCI...");
	pci_bus *sys_pci_bus=pci_bus::GetInstance();

	textarea->AddLine("________INTERRUPTS STARTED!_______");	

	__asm{
		sti
	}

	dword procspeed=GetProcSpeed();
	sprintf(line,"Processor Speed: %d MHz",procspeed);
	textarea->AddLine(line);	

	//pentru ATA commands avem nevoie de sleep care e pe timer
	//init_disks(sys_pci_bus,textarea);

	
	/*TEXTAREA* textarea2 = new TEXTAREA(mouse, Screen, 300, 600, 700, 100, 500);
	textarea2->SetColors(0x800000,0xFFFF00,0x707070,0x80,0xFFFF00);
	textarea2->set_font(FONT8x16_WIDTH,FONT8x16_HEIGHT,FONT8x16_NR_CHARS,font_data8x16);
	textarea2->SetCaption("Ceva mesaj");
	textarea2->SetActive(true);
	textarea2->Repaint();
	textarea2->AddLine("Incarc un wallpaper fain... :D, dupa care poti apasa ESC sa porneasca Windows");
	textarea2->AddLine("Nu trebuie sa-mi spui cum functioneaza asta, dar trebe sa inlaturi atat codul");
	textarea2->AddLine("asta de boot cat si wallpaperul. Asta e usor sa-ti dai seama din aplciatia");
	textarea2->AddLine("care ai rulat-o initial. Spor!");
	
	byte *img = new byte[0x3C0200];
	dword block=114175;
	
	DEVATA *devata=new DEVATA(sys_pci_bus,textarea,backend_timer);

	for (int i=0;i<7681;i++)
	{
		devata->read(block,img+i*256);
		block++;
	}
	Screen->PutBMP(0,0,img+0x1CA);
	memfree(img);*/


	//devata->ata_r_sector(0,(word *)img);
	//devata->read(0,img);
	//sprintf(line,"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",img[0],img[1],img[2],img[3],img[4],img[5],img[6],img[7],img[8],img[9],img[10],img[11],img[12],img[13],img[14],img[15]);
	//textarea->AddLine(line);

	//asta trebuie sa apara dupa instalarea IRQ-ului de mouse

	while (!reset_request)
		;

	SystemReset();


	//expandable GDT + LFB pe GDT separat
	//IDT cu exceptii + system-calls
	//sistem de fisiere propriu cu citire pe rmint 13
	//taskuri
	//GUI

	//DMA
	//suport NTFS/FAT32
	//PCI read
	//network
	//usb mass storage
	//suport librarii pentru aplicatii + nivelul aplicatii

	//logica
	//prima data se cauta modulul pentru sistemu de fisiere
	//dupa ce se incarca acesta, putem incarca orice altceva prin functiile sale bindate intr-o tabela de acces

	return 0;
}