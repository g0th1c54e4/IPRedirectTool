#ifndef WFP_REDIRECT_NET
#define WFP_REDIRECT_NET


typedef unsigned short int uint16;
typedef unsigned long int uint32;

// 短整型大小端互换
#define BigLittleSwap16(A)  ((((uint16)(A) & 0xff00) >> 8) | (((uint16)(A) & 0x00ff) << 8))
// 长整型大小端互换
#define BigLittleSwap32(A)  ((((uint32)(A) & 0xff000000) >> 24) | (((uint32)(A) & 0x00ff0000) >> 8) | (((uint32)(A) & 0x0000ff00) << 8) | (((uint32)(A) & 0x000000ff) << 24))

unsigned short int NtoHs(unsigned short int n);
unsigned short int HtoNs(unsigned short int h);
unsigned long int HtoNl(unsigned long int h);
unsigned long int NtoHl(unsigned long int n);
int checkCPUendian();



#endif