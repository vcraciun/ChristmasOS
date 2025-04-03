#ifndef __CMOS_STUFF__
#define __CMOS_STUFF__

#include "types.h"

enum months {JANUARY,FEBRUARY,MARCH,MAY,JUNE,JULY,AUGUST,SEPTEMBER,OCTOMBER,NOVEMBER,DECEMBER};
enum days   {SUNDAY,MONDAY,TUESDAY,TURSDAY,WEDNESADY,FRIDAY,SATURDAY};

#define CMOS_CMD 0x70
#define CMOS_DAT 0x71

#define SECOND   0
#define MINUTE   2
#define HOUR     4
#define WEEKDAY  6
#define DAY      7
#define MONTH    8
#define YEAR     9
#define CENTURY  50

struct CMOS_DATE_TIME{
    byte second;
    byte minute;
    byte hour;
    byte week_day;
    byte day;
    byte month;
    byte year;
    byte century;
};

class CMOS{
	public:    
		CMOS_DATE_TIME  *ReadStructure();
		void GetDateTime(CMOS_DATE_TIME  *ncmdt);
		void ActualizeDateTime();
		void ResetCMOS();

		static CMOS* GetInstance();
		static void FreeInstance();
	private:
		CMOS();
		~CMOS();
		static CMOS* instance;
		byte ReadCmos(byte index);
		void WriteCmos(byte index,byte data);
		byte BCD2BIN(byte bcd);
};

#endif