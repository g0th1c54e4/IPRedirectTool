#include "net.h"

int checkCPUendian() {

	union {
		unsigned long int i;

		unsigned char s[4];

	}c;

	c.i = 0x12345678;

	return (0x12 == c.s[0]);

}

// 模拟htonl函数，本机字节序转网络字节序 32Bit
unsigned long int HtoNl(unsigned long int h) {
	return checkCPUendian() ? h : BigLittleSwap32(h);
}

// 模拟ntohl函数，网络字节序转本机字节序 32Bit
unsigned long int NtoHl(unsigned long int n) {
	return checkCPUendian() ? n : BigLittleSwap32(n);
}

// 模拟htons函数，本机字节序转网络字节序 16Bit
unsigned short int HtoNs(unsigned short int h) {
	return checkCPUendian() ? h : BigLittleSwap16(h);
}

// 模拟ntohs函数，网络字节序转本机字节序 16Bit
unsigned short int NtoHs(unsigned short int n) {
	return checkCPUendian() ? n : BigLittleSwap16(n);
}