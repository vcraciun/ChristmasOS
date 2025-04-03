#ifndef __COMMON__
#define __COMMON__

#ifdef __cplusplus
extern "C" {
#endif
	
int DoRmInt(int intr,int ax,int bx,int cx,int dx); 
void gdt_flush();

#ifdef __cplusplus
}
#endif 

#endif