#include "net.h"

int checkCPUendian() {

	union {
		unsigned long int i;

		unsigned char s[4];

	}c;

	c.i = 0x12345678;

	return (0x12 == c.s[0]);

}

// ģ��htonl�����������ֽ���ת�����ֽ��� 32Bit
unsigned long int HtoNl(unsigned long int h) {
	return checkCPUendian() ? h : BigLittleSwap32(h);
}

// ģ��ntohl�����������ֽ���ת�����ֽ��� 32Bit
unsigned long int NtoHl(unsigned long int n) {
	return checkCPUendian() ? n : BigLittleSwap32(n);
}

// ģ��htons�����������ֽ���ת�����ֽ��� 16Bit
unsigned short int HtoNs(unsigned short int h) {
	return checkCPUendian() ? h : BigLittleSwap16(h);
}

// ģ��ntohs�����������ֽ���ת�����ֽ��� 16Bit
unsigned short int NtoHs(unsigned short int n) {
	return checkCPUendian() ? n : BigLittleSwap16(n);
}