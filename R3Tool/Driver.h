#pragma once

#define DEVICE_NAME L"\\Device\\LingMoDev"
#define SYMBOL_NAME L"\\??\\LingMoSym"

#define FILE_DEVICE_UNKNOWN 0x00000022
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_UPDATE_REDIRECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, 0, FILE_ALL_ACCESS)


typedef struct _UpdateRedirect {
	DWORD dwUserTargetIPAddress;
	WORD wdUserTargetPort;
	DWORD dwUserLocalIPAddress;
	WORD wdUserLocalPort;
} UpdateRedirect;