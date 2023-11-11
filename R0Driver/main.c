#define NDIS_SUPPORT_NDIS6 1
#include <ntifs.h>
#include <fwpmk.h>
#include <fwpsk.h>
#include <stdio.h>
#include <intrin.h>
#include "main.h"
#include "net.h"
#include "IOCTL.h"


PDEVICE_OBJECT pDevice = NULL;
HANDLE hFwpmEngine = 0;
UINT32 ul32CallOutId = 0;
UINT64 ul64FilterId = 0;

VOID DriverUnload(PDRIVER_OBJECT pDriverObject);

VOID WfpLoad(GUID* layerKey);
VOID WfpUnload();

VOID NTAPI classifyFunc_Redirect( // FWPM_LAYER_ALE_CONNECT_REDIRECT_V4
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
);
VOID NTAPI classifyFunc_Block( // FWPM_LAYER_ALE_AUTH_CONNECT_V4
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
);

VOID NTAPI classifyFunc_StreamWrite( // FWPM_LAYER_STREAM_V4
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
);

NTSTATUS NTAPI notifyFunc(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER* filter
);


VOID NTAPI flowDeleteFunc(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
);

// {4D7387FA-6F3B-40DD-B3A2-E90CA3BB2808}
static const GUID Guidkey = { 0x4D7387FA, 0x6F3b, 0x40dd, { 0xb3, 0xa2, 0xe9, 0xc, 0xa3, 0xbb, 0x28, 0x8 } };

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pustrRegistryPath) {
	DbgPrint("[LingMo Driver]Load\n");
	if (NT_SUCCESS(Device_Create(pDriverObject))) {
		DbgPrint("[LingMo Driver]Device Create Successful\n");
	}
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		pDriverObject->MajorFunction[i] = DeviceIrpControl;
	}

	pDriverObject->DriverUnload = DriverUnload;

	//拦截和放行数据包使用     &FWPM_LAYER_ALE_AUTH_CONNECT_V4
	//修改IP和端口号使用     &FWPM_LAYER_ALE_CONNECT_REDIRECT_V4
	WfpLoad(&FWPM_LAYER_ALE_CONNECT_REDIRECT_V4);


	return STATUS_SUCCESS;
}

VOID WfpLoad(GUID* layerKey) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	FWPM_SESSION session;
	memset(&session, 0, sizeof(FWPM_SESSION));
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;

	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &hFwpmEngine);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpmEngineOpen() Failed:0x%X\n", status);
		return;
	}

	status = FwpmTransactionBegin(hFwpmEngine, 0);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpmTransactionBegin() Failed:0x%X\n", status);
		return;
	}

	FWPS_CALLOUT fwpsCallout;
	memset(&fwpsCallout, 0, sizeof(FWPS_CALLOUT));
	fwpsCallout.calloutKey = Guidkey;
	if (IsEqualGUID(layerKey, &FWPM_LAYER_ALE_CONNECT_REDIRECT_V4)) {
		fwpsCallout.classifyFn = classifyFunc_Redirect;
	}
	if (IsEqualGUID(layerKey, &FWPM_LAYER_ALE_AUTH_CONNECT_V4)) {
		fwpsCallout.classifyFn = classifyFunc_Block;
	}
	if (IsEqualGUID(layerKey, &FWPM_LAYER_STREAM_V4)) {
		fwpsCallout.classifyFn = classifyFunc_StreamWrite;
	}
	fwpsCallout.flowDeleteFn = flowDeleteFunc;
	fwpsCallout.notifyFn = notifyFunc;

	status = FwpsCalloutRegister(pDevice, &fwpsCallout, &ul32CallOutId);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpsCalloutRegister() Failed:0x%X\n", status);
		return;
	}

	FWPM_CALLOUT fwpmCallOut;
	memset(&fwpmCallOut, 0, sizeof(FWPM_CALLOUT));
	fwpmCallOut.calloutKey = Guidkey;
	fwpmCallOut.calloutId = ul32CallOutId;
	FWPM_DISPLAY_DATA fwpmDisplayData;
	memset(&fwpmDisplayData, 0, sizeof(FWPM_DISPLAY_DATA));
	fwpmDisplayData.name = L"LingMoWfp";
	fwpmDisplayData.description = L"LingMoWfp By LingMo";
	fwpmCallOut.displayData = fwpmDisplayData;
	fwpmCallOut.applicableLayer = *layerKey;

	status = FwpmCalloutAdd(hFwpmEngine, &fwpmCallOut, NULL, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpmCalloutAdd() Failed:0x%X\n", status);
		return;
	}

	FWPM_FILTER fwpmFilter;
	memset(&fwpmFilter, 0, sizeof(FWPM_FILTER));
	fwpmDisplayData.name = L"LingMoWfp";
	fwpmDisplayData.description = L"LingMoWfp By LingMo";
	fwpmFilter.displayData = fwpmDisplayData;
	fwpmFilter.action.calloutKey = Guidkey;
	fwpmFilter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	fwpmFilter.weight.type = FWP_EMPTY;
	//FWPM_LAYER_ALE_AUTH_CONNECT_V4
	fwpmFilter.layerKey = *layerKey;
	fwpmFilter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;


	status = FwpmFilterAdd(hFwpmEngine, &fwpmFilter, NULL, &ul64FilterId);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpmFilterAdd() Failed:0x%X\n", status);
		return;
	}

	status = FwpmTransactionCommit(hFwpmEngine);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpmTransactionCommit() Failed:0x%X\n", status);
		return;
	}

}


VOID WfpUnload() {
	if (hFwpmEngine != NULL) {
		FwpmFilterDeleteById(hFwpmEngine, ul64FilterId);
		FwpmCalloutDeleteById(hFwpmEngine, ul32CallOutId);
		FwpsCalloutUnregisterById(ul32CallOutId);
		FwpmEngineClose(hFwpmEngine);
	}
	return;
}


VOID DriverUnload(PDRIVER_OBJECT pDriverObject) {
	if (NT_SUCCESS(Device_Close(pDriverObject))) {
		DbgPrint("[LingMo Driver]Device Delete Successful\n");
	}
	WfpUnload();

	UNICODE_STRING ustrSymName;
	RtlInitUnicodeString(&ustrSymName, SYMBOL_NAME);
	IoDeleteSymbolicLink(&ustrSymName);
	if (pDriverObject->DeviceObject) {
		IoDeleteDevice(pDriverObject->DeviceObject);
	}

	DbgPrint("[LingMo Driver]Unload\n");
}

NTSTATUS Device_Create(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING ustrDeviceName = { 0 };
	RtlInitUnicodeString(&ustrDeviceName, DEVICE_NAME);
	UNICODE_STRING ustrSymbolName = { 0 };
	RtlInitUnicodeString(&ustrSymbolName, SYMBOL_NAME);
	status = IoCreateDevice(pDriverObject, NULL, &ustrDeviceName, FILE_DEVICE_UNKNOWN, NULL, TRUE, &pDevice);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]Device Create Failed:0x%x\n", status);
		goto end;
	}
	DbgPrint("[LingMo Driver]已成功创建驱动设备.\n");
	status = IoCreateSymbolicLink(&ustrSymbolName, &ustrDeviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]SymbolicLink Create Failed:0x%x\n", status);
		goto end;
	}
	status = STATUS_SUCCESS;
	DbgPrint("[LingMo Driver]已成功创建符号链接.\n");
end:
	return status;
}

NTSTATUS Device_Close(PDRIVER_OBJECT pDriverObject) {
	NTSTATUS status = STATUS_SUCCESS;
	if (pDriverObject->DeviceObject) {
		UNICODE_STRING ustrSymbolName = { 0 };
		RtlInitUnicodeString(&ustrSymbolName, SYMBOL_NAME);
		status = IoDeleteSymbolicLink(&ustrSymbolName);
		if (!NT_SUCCESS(status)) {
			DbgPrint("[LingMo Driver]SymbolicLink Delete Failed:0x%x\n", status);
			status = STATUS_UNSUCCESSFUL;
		}
		DbgPrint("[LingMo Driver]已成功删除符号链接.\n");
		IoDeleteDevice(pDriverObject->DeviceObject);
	}
	return status;
}

NTSTATUS DeviceIrpControl(PDEVICE_OBJECT pDevice, PIRP pIrp) {
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pBuffer = pIrp->AssociatedIrp.SystemBuffer;
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG dwReadBytes = NULL;
	switch (pStack->MajorFunction) {
	case IRP_MJ_CREATE:

		pIrp->IoStatus.Information = 0;
		break;
	case IRP_MJ_CLOSE:

		pIrp->IoStatus.Information = 0;
		break;
	case IRP_MJ_DEVICE_CONTROL:
		switch (pStack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_UPDATE_REDIRECT:
			status = IoControl_UpdateRedirect(pBuffer, &dwReadBytes);
			pIrp->IoStatus.Information = dwReadBytes;
			break;
		default:
			pIrp->IoStatus.Information = 0;
			break;
		}
		break;
	default:
		pIrp->IoStatus.Information = 0;
		break;
	}
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID NTAPI classifyFunc_Redirect(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;


	if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0) {
		DbgPrint("[LingMo Driver]No Write Privilege\n");
		return;
	}

	UNREFERENCED_PARAMETER(inFixedValues);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(layerData);

	HANDLE hRedirect = 0;
	status = FwpsRedirectHandleCreate(&Guidkey, 0, &hRedirect);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpsRedirectHandleCreate() Failed:0x%X\n", status);
		return;
	}
	FWPS_CONNECTION_REDIRECT_STATE fwpsRedirectState = FwpsQueryConnectionRedirectState(inMetaValues->redirectRecords, hRedirect, NULL);

	if (fwpsRedirectState != FWPS_CONNECTION_NOT_REDIRECTED) {
		DbgPrint("Redirect: Connection was already redirected (presumably by us). Ignoring it.\n");
		return;
	}

	UNREFERENCED_PARAMETER(inMetaValues);

	UINT64 ul64ClassifyHandle = 0;
	status = FwpsAcquireClassifyHandle((LPVOID)classifyContext, (UINT32)0, &ul64ClassifyHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpsAcquireClassifyHandle() Failed:0x%X\n", status);
		return;
	}
	FWPS_CONNECT_REQUEST0* connectRequest = 0;
	status = FwpsAcquireWritableLayerDataPointer(ul64ClassifyHandle, filter->filterId, 0, &connectRequest, classifyOut);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[LingMo Driver]FwpsAcquireWritableLayerDataPointer() Failed:0x%X\n", status);
		return;
	}

	SOCKADDR_IN* remoteAddr = (SOCKADDR_IN*)&connectRequest->remoteAddressAndPort;
	SOCKADDR_IN* localAddr = (SOCKADDR_IN*)&connectRequest->localAddressAndPort;

	UINT32 remoteAddrIp = NtoHl(remoteAddr->sin_addr.S_un.S_addr);
	UINT16 remoteAddrPort = NtoHs(remoteAddr->sin_port);

	//欲重定向的目标IP及端口
	if ((remoteAddrIp == dwUserTargetIPAddress) && (remoteAddrPort == wdUserTargetPort)) {

		//要重定向的端口号
		remoteAddr->sin_port = HtoNs(wdUserLocalPort);

		//要重定向的IP地址
		remoteAddr->sin_addr.S_un.S_addr = HtoNl(dwUserLocalIPAddress);

	}

	connectRequest->localRedirectHandle = hRedirect;    //根据微软文档所述,这句话需要执行
	connectRequest->localRedirectTargetPID = 0xFFFF;    //根据微软文档所述,这句话需要执行
	FwpsApplyModifiedLayerData(ul64ClassifyHandle, connectRequest, 0);
	if (ul64ClassifyHandle != NULL) {
		FwpsReleaseClassifyHandle(ul64ClassifyHandle);
	}
	FwpsRedirectHandleDestroy(hRedirect);


	// - - - - - - -  -- - - - - - -- -- - - --- -- - - -
	classifyOut->rights = classifyOut->rights & (~FWPS_RIGHT_ACTION_WRITE);
	classifyOut->flags = classifyOut->flags | FWPS_CLASSIFY_OUT_FLAG_ABSORB;
	classifyOut->actionType = FWP_ACTION_PERMIT;
}

NTSTATUS NTAPI notifyFunc(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER* filter
) {
	return STATUS_SUCCESS;
}


VOID NTAPI flowDeleteFunc(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
) {
	return;
}


VOID NTAPI classifyFunc_Block(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
) {

	// 数据包的方向,取值 FWP_DIRECTION_INBOUND = 1 或 FWP_DIRECTION_OUTBOUND = 0
	WORD wDirection = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_DIRECTION].value.int8;

	// 定义本机地址与本机端口
	ULONG ulLocalIp = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS].value.uint32;
	UINT16 uLocalPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT].value.uint16;

	// 定义对端地址与对端端口
	ULONG ulRemoteIp = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS].value.uint32;
	UINT16 uRemotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT].value.uint16;

	// 获取进程ID
	ULONG64 processId = inMetaValues->processId;
	UCHAR szProcessPath[256] = { 0 };
	RtlZeroMemory(szProcessPath, 256);

	// 获取进程路径
	for (ULONG i = 0; i < inMetaValues->processPath->size; i++) {
		// 里面是宽字符存储的
		szProcessPath[i] = inMetaValues->processPath->data[i];
	}

	// 设置默认规则 允许连接
	classifyOut->actionType = FWP_ACTION_PERMIT;

	// 输出对端地址字符串 并阻断链接
	char szRemoteAddress[256] = { 0 };
	char szRemotePort[128] = { 0 };

	sprintf(szRemoteAddress, "%u.%u.%u.%u", (ulRemoteIp >> 24) & 0xFF, (ulRemoteIp >> 16) & 0xFF, (ulRemoteIp >> 8) & 0xFF, (ulRemoteIp) & 0xFF);
	sprintf(szRemotePort, "%d", uRemotePort);


	if (strcmp(szRemoteAddress, "110.42.213.115") == 0 && strcmp(szRemotePort, "3000") == 0) {

		// 拒绝连接 110.42.213.115
		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights = classifyOut->rights & (~FWPS_RIGHT_ACTION_WRITE);
		classifyOut->flags = classifyOut->flags | FWPS_CLASSIFY_OUT_FLAG_ABSORB;

	}

	if (wcsstr((PWCHAR)szProcessPath, L"QQPlus.exe") != 0) {

		// 拒绝进程为 QQPlus.exe 的所有连接
		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights = classifyOut->rights & (~FWPS_RIGHT_ACTION_WRITE);
		classifyOut->flags = classifyOut->flags | FWPS_CLASSIFY_OUT_FLAG_ABSORB;

	}

	// 显示
	DbgPrint("方向: %d -> 本端地址: %u.%u.%u.%u:%d -> 对端地址: %u.%u.%u.%u:%d -> 进程ID: %I64d -> 路径: %S \n",
		wDirection,
		(ulLocalIp >> 24) & 0xFF,
		(ulLocalIp >> 16) & 0xFF,
		(ulLocalIp >> 8) & 0xFF,
		(ulLocalIp) & 0xFF,
		uLocalPort,
		(ulRemoteIp >> 24) & 0xFF,
		(ulRemoteIp >> 16) & 0xFF,
		(ulRemoteIp >> 8) & 0xFF,
		(ulRemoteIp) & 0xFF,
		uRemotePort,
		processId,
		(PWCHAR)szProcessPath);


}


VOID NTAPI classifyFunc_StreamWrite( // FWPM_LAYER_STREAM_V4
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
) {
	if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
		DbgPrint("[LingMoDriver]Stream Write No Privilege.\n");
		return;
	}
	FWPS_STREAM_CALLOUT_IO_PACKET* pStreamPacket = (FWPS_STREAM_CALLOUT_IO_PACKET*)layerData;
	ULONG ulRemoteIp = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS].value.uint32;

	if ((pStreamPacket) && (pStreamPacket->streamData != 0) && (pStreamPacket->streamData->dataLength != 0)) {

		DbgPrint("[LingMoDriver]侦测到流量流动! 0x%X\n", ulRemoteIp);

		DWORD64 streamLength = pStreamPacket->streamData->dataLength;

		BOOLEAN inbound = (BOOLEAN)((pStreamPacket->streamData->flags & FWPS_STREAM_FLAG_RECEIVE) == FWPS_STREAM_FLAG_RECEIVE);
		BYTE* stream = ExAllocatePoolWithTag(NonPagedPool, streamLength, 0xFACE0001); //此处的TAG数值可自行拟定
		DWORD64 byte_copied = 0;
		if (stream != NULL) {
			RtlZeroMemory(stream, streamLength);
			FwpsCopyStreamDataToBuffer(
				pStreamPacket->streamData,
				stream,
				streamLength,
				&byte_copied);


			//未写完

			ExFreePool(stream);
		}

	}
	classifyOut->actionType = FWP_ACTION_CONTINUE;
}


NTSTATUS IoControl_UpdateRedirect(UpdateRedirect* pUpdateRedirectInfo, ULONG* pdwReadBytes) {

	dwUserTargetIPAddress = pUpdateRedirectInfo->dwUserTargetIPAddress;
	wdUserTargetPort = pUpdateRedirectInfo->wdUserTargetPort;
	dwUserLocalIPAddress = pUpdateRedirectInfo->dwUserLocalIPAddress;
	wdUserLocalPort = pUpdateRedirectInfo->wdUserLocalPort;


	return STATUS_SUCCESS;
}