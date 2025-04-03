#include "cmos.h"
#include "system.h"

CMOS* CMOS::instance = 0;

CMOS* CMOS::GetInstance()
{
    if (!instance)
        instance = new CMOS;
    return instance;
}

void CMOS::FreeInstance()
{
    delete instance;
}

CMOS::CMOS()
{
}

CMOS::~CMOS(){}

byte CMOS::ReadCmos(byte index)
{
    byte data;
    
    outb(0x70,index);
    data=inb(0x71);
    
    return data;
}
    
void CMOS::WriteCmos(byte index,byte data)
{
    outb(0x70,index);
    outb(0x71,data);
}

void CMOS::ResetCMOS()
{
    byte i;
    
    for (i=0;i<128;i++)
        WriteCmos(i,0);
}

void CMOS::GetDateTime(CMOS_DATE_TIME *ncmdt)
{
    ncmdt->second=BCD2BIN(ReadCmos(SECOND));
    ncmdt->minute=BCD2BIN(ReadCmos(MINUTE));
    ncmdt->hour=BCD2BIN(ReadCmos(HOUR));
    ncmdt->week_day=BCD2BIN(ReadCmos(WEEKDAY));
    ncmdt->day=BCD2BIN(ReadCmos(DAY));
    ncmdt->month=BCD2BIN(ReadCmos(MONTH));
    ncmdt->year=BCD2BIN(ReadCmos(YEAR));
    ncmdt->century=BCD2BIN(ReadCmos(CENTURY));
}

byte CMOS::BCD2BIN(byte bcd)
{
    return ((bcd>>4)*10)+(bcd&0x0F);
}

