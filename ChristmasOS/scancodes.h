#ifndef __SCANCODES__
#define __SCANCODES__

//scancodes
#define RELEASE  0x80
#define EXTSCAN1 0xE0
#define EXTSCAN2 0xE1

#define CAPLOCK_DOWN 0x3A
#define NUMLOCK_DOWN 0x45
#define SCRLOCK_DOWN 0x46
#define ALT_DOWN     0x38
#define CTRL_DOWN    0x1D
#define LSHIFT_DOWN  0x2A
#define RSHIFT_DOWN  0x36
#define F6_DOWN      0x40
#define F5_DOWN      0x3F
#define F4_DOWN      0x3E
#define F3_DOWN      0x3D
#define F2_DOWN      0x3C
#define F1_DOWN      0x3B
#define F12_DOWN     0x58
#define F11_DOWN     0x57
#define F10_DOWN     0x44
#define F9_DOWN      0x43
#define F8_DOWN      0x42
#define F7_DOWN      0x41
#define INSERT_DOWN  0x52
#define DELETE_DOWN  0x53
#define HOME_DOWN    0x47
#define END_DOWN     0x4F
#define PGUP_DOWN    0x49
#define PGDN_DOWN    0x51
#define UP_DOWN      0x48
#define DOWN_DOWN    0x50
#define LEFT_DOWN    0x4B
#define RIGHT_DOWN   0x4D
#define WIN_DOWN     0x5B
#define BREAK_DOWN1  0x1D
#define BREAK_DOWN2  0x45

#define CAPLOCK_UP   CAPLOCK_DOWN | RELEASE
#define NUMLOCK_UP   NUMLOCK_DOWN | RELEASE
#define SCRLOCK_UP   SCRLOCK_DOWN | RELEASE
#define ALT_UP       ALT_DOWN | RELEASE
#define CTRL_UP      CTRL_DOWN | RELEASE
#define LSHIFT_UP    LSHIFT_DOWN | RELEASE
#define RSHIFT_UP    RSHIFT_DOWN | RELEASE
#define F1_UP        F1_DOWN | RELEASE
#define F2_UP        F2_DOWN | RELEASE
#define F3_UP        F3_DOWN | RELEASE
#define F4_UP        F4_DOWN | RELEASE
#define F5_UP        F5_DOWN | RELEASE
#define F6_UP        F6_DOWN | RELEASE
#define F7_UP        F7_DOWN | RELEASE
#define F8_UP        F8_DOWN | RELEASE
#define F9_UP        F9_DOWN | RELEASE
#define F10_UP       F10_DOWN | RELEASE
#define F11_UP       F11_DOWN | RELEASE
#define F12_UP       F12_DOWN | RELEASE
#define INSERT_UP    INSERT_DOWN | RELEASE
#define DELETE_UP    DELETE_DOWN | RELEASE
#define HOME_UP      HOME_DOWN | RELEASE
#define END_UP       END_DOWN | RELEASE
#define PGUP_UP      PGUP_DOWN | RELEASE
#define PGDN_UP      PGDN_DOWN | RELEASE
#define UP_UP        UP_DOWN | RELEASE
#define DOWN_UP      DOWN_DOWN | RELEASE
#define LEFT_UP      LEFT_DOWN | RELEASE
#define RIGHT_UP     RIGHT_DOWN | RELEASE
#define WIN_UP       WIN_DOWN | RELEASE
#define BREAK_UP1    BREAK_DOWN1 | RELEASE
#define BREAK_UP2    BREAK_DOWN2 | RELEASE

#define STATUS_PORT  0x64
#define COMMAND_PORT 0x64
#define DATA_PORT    0x60
#define CMD_PORT60   0x60
#define PPI          0x61

#define STATUS_OUTBUF   0x01
#define STATUS_INBUF    0x02
#define STATUS_SYSFLG   0x04
#define STATUS_CDAVAIL  0x08
#define STATUS_KBACTIVE 0x10
#define STATUS_ERROR    0x20
#define STATUS_TIMEOUT  0x40
#define STATUS_PARITY   0x80

#define CMD_CMDASSCAN     0x20
#define CMD_STORECMD      0x60
#define CMD_DISABLEMICE   0xA7
#define CMD_ENABLEMICE    0xA8
#define CMD_TESTMICE      0xA9
#define CMD_DISABLEKBD    0xAD
#define CMD_ENABLE_KBD    0xAE
#define CMD_WRITEKBBUFF   0xD2
#define CMD_WRITEMICEBUFF 0xD3
#define CMD_WRITEMICEDATA 0xD4

#define CMD_B_KEYBINT     0x01
#define CMD_B_MICEINT     0x02
#define CMD_B_SYSFLG      0x04
#define CMD_B_KEYBDIS     0x10
#define CMD_B_MICEDIS     0x20

#define CMD_SET_SCALE21      0xE7
#define CMD_SET_SCALE11      0xE6
#define CMD_SET_RESOLUTION   0xE8
#define CMD_SET_STREAM_MODE  0xEA
#define CMD_UPDATE_LEDS      0xED
#define CMD_SELECT_SCANCODES 0xF0
#define CMD_GET_DEVICE_ID    0xF2
#define CMD_TYPEMATIC_RATE   0xF3
#define CMD_ENABLE_KEYBOARD  0xF4
#define CMD_RESET_WAIT       0xF5
#define CMD_RESET_SCAN       0xF6
#define CMD_USE_DEFAULTS     0xF6
#define CMD_AUTO_REPEAT      0xF7
#define CMD_GENUPDOWN        0xF8
#define CMD_GENUPONLY        0xF9
#define CMD_REPEATUPDOWN     0xFA
#define CMD_REPEATKEY        0xFB
#define CMD_GENUPDOWNKEY     0xFC
#define CMD_GENDOWNKEY       0xFD
#define CMD_RESENDRESULT     0xFE
#define CMD_RESETTOPOST      0xFF

#endif