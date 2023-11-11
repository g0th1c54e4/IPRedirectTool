#pragma once
#include <ntifs.h>

#define DEVICE_NAME L"\\Device\\LingMoDev"
#define SYMBOL_NAME L"\\??\\LingMoSym"

VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS Device_Create(PDRIVER_OBJECT pDriverObject);
NTSTATUS Device_Close(PDRIVER_OBJECT pDriverObject);
NTSTATUS DeviceIrpControl(PDEVICE_OBJECT pDevice, PIRP pIrp);


typedef struct _UpdateRedirect {
	DWORD dwUserTargetIPAddress;
	WORD wdUserTargetPort;
	DWORD dwUserLocalIPAddress;
	WORD wdUserLocalPort;
} UpdateRedirect;
NTSTATUS IoControl_UpdateRedirect(UpdateRedirect* prwMEMORY, ULONG* pdwReadBytes);

// 以下是R3层反馈的数据

DWORD dwUserTargetIPAddress = 0x00000000;
WORD wdUserTargetPort = 0x0000;
DWORD dwUserLocalIPAddress = 0x00000000;
WORD wdUserLocalPort = 0x0000;