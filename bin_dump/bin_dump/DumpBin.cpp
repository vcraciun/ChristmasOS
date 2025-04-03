#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#pragma warning (disable:4996)

int main(int argc,char *argv[])
{
	HANDLE hf;
	BYTE *buffer;
	DWORD read,fsize;
	char binname[256],*extloc;
	char appname[256]={0};
	BYTE *mapped;
	int i,j;
	IMAGE_NT_HEADERS *imnthdr;
	IMAGE_SECTION_HEADER *imsechdr;

	appname[255]=0;
	j=254;
	for (i=strlen(argv[0])-1;i>0;i--,j--)
		if (argv[0][i]!='\\')
			appname[j]=argv[0][i];
		else
			break;
	j++;
	if (argc<2)
	{
		printf("Invlaid command line\nCommand line example: %s target_exe\n",appname+j);
		return 0;
	}

	hf=CreateFile(argv[1],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	fsize=GetFileSize(hf,NULL);
	buffer=(BYTE*)VirtualAlloc(NULL,fsize,MEM_COMMIT,PAGE_READWRITE);
	memset(buffer,0,fsize);
	ReadFile(hf,buffer,fsize,&read,NULL);
	CloseHandle(hf);

	memset(binname,0,256);
	extloc=strstr(argv[1],".exe");
	strncpy(binname,argv[1],(DWORD)(extloc-argv[1]));
	strcat(binname,".bin");
	hf=CreateFile(binname,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	imnthdr=(IMAGE_NT_HEADERS*)(buffer+*(DWORD*)(buffer+0x3C));
	imsechdr=(IMAGE_SECTION_HEADER*)(buffer+0x18+*(DWORD*)(buffer+0x3C)+imnthdr->FileHeader.SizeOfOptionalHeader);
	mapped=(BYTE*)VirtualAlloc(NULL,0x50000,MEM_COMMIT,PAGE_READWRITE);
	memset(mapped,0,0x50000);
	j=0;
	for (i=0;i<imnthdr->FileHeader.NumberOfSections;i++)
	{
		j=imsechdr[i].VirtualAddress;
		memcpy(mapped+j,buffer+j,imsechdr[i].Misc.VirtualSize);
		j+=imsechdr[i].SizeOfRawData;
	}
	WriteFile(hf,mapped,j,&read,NULL);
	CloseHandle(hf);

	VirtualFree(buffer,fsize,MEM_FREE);
	VirtualFree(mapped,j,MEM_FREE);

	printf("Done Writing Binary Data to [%s]\n",binname);

	return 0;
}