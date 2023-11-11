
// R3ToolDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "R3Tool.h"
#include "R3ToolDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CR3ToolDlg 对话框



CR3ToolDlg::CR3ToolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_R3TOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CR3ToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CR3ToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_UNLOAD_DRIVER, &CR3ToolDlg::OnBnClickedButtonUnloadDriver)
	ON_BN_CLICKED(IDC_BUTTON_REDIRECT_SWITCH, &CR3ToolDlg::OnBnClickedButtonRedirectSwitch)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON_INSTALL_DRIVER, &CR3ToolDlg::OnBnClickedButtonInstallDriver)
	ON_BN_CLICKED(IDC_BUTTON_UNINSTALL_DRIVER, &CR3ToolDlg::OnBnClickedButtonUninstallDriver)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_DRIVER, &CR3ToolDlg::OnBnClickedButtonLoadDriver)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CR3ToolDlg 消息处理程序

BOOL CR3ToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CR3ToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CR3ToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CR3ToolDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}

void CR3ToolDlg::OnBnClickedButtonInstallDriver() { //安装驱动
	if (bDriverIsInstall == TRUE) {
		MessageBox(TEXT("因为驱动服务已经存在,所以驱动服务安装失败"), TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}

	if (hServer != NULL) {
		CloseServiceHandle(hServer);
		return;
	}

	SC_HANDLE hScm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScm == NULL) {
		CString ErrorMsg = L"";
		ErrorMsg.Format(L"服务控制管理器打开失败!错误代码:%d", GetLastError());
		MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}

	WCHAR wstrServerName[255] = L"LingMo_Redirect";
	
	WCHAR wstrCurrentDirectoryPath[MAX_PATH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH, wstrCurrentDirectoryPath);
	CString cstrDriverFilePath = wstrCurrentDirectoryPath;
	cstrDriverFilePath = cstrDriverFilePath + L"\\Redirect.sys";

	hServer = CreateServiceW(hScm, wstrServerName, wstrServerName, SC_MANAGER_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, cstrDriverFilePath, 0, 0, 0, 0, 0);
	if (hServer == NULL) {
		DWORD dwErrorCode = GetLastError();
		if (dwErrorCode == ERROR_SERVICE_EXISTS || dwErrorCode == ERROR_ACCESS_DENIED) {
			MessageBox(L"服务已存在,创建失败!", TEXT("IPRedirectTool:"), MB_ICONERROR);
			hServer = OpenServiceW(hScm, wstrServerName, SERVICE_ALL_ACCESS);
			if (hServer == NULL) {
				CloseServiceHandle(hScm);
				MessageBox(L"服务已存在,但获取失败!", TEXT("IPRedirectTool:"), MB_ICONERROR);
				return;
			}
		}
		else {
			CString ErrorMsg = L"";
			ErrorMsg.Format(L"服务打开失败!错误代码:%d", GetLastError());
			MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
			CloseServiceHandle(hScm);
			return;
		}
	}
	if (hScm != NULL) {
		CloseServiceHandle(hScm);
	}

	bDriverIsInstall = TRUE;
	MessageBox(TEXT("安装成功"), TEXT("IPRedirectTool:"), MB_OK);
	return;
}

void CR3ToolDlg::OnBnClickedButtonLoadDriver() { //加载驱动
	if (bDriverIsLoad == TRUE) {
		MessageBox(TEXT("因为驱动服务已经启动,所以驱动服务启动失败"), TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}
	if (hServer == NULL) {
		MessageBox(L"无服务句柄!启动失败!", TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}

	BOOL bServrStartStatus = StartServiceW(hServer, NULL, NULL);
	if (bServrStartStatus == NULL) {
		DWORD dwErrorCode = GetLastError();
		if (dwErrorCode == ERROR_SERVICE_ALREADY_RUNNING) {
			MessageBox(L"服务运行中,启动失败!", TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}
		else if (dwErrorCode == ERROR_SERVICE_REQUEST_TIMEOUT) {
			MessageBox(L"服务挂起中,启动失败!", TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}
		else {
			CString ErrorMsg = L"";
			ErrorMsg.Format(L"服务启动失败!错误代码:%d", GetLastError());
			MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}

	}
	bDriverIsLoad = TRUE;
	MessageBox(TEXT("加载成功"), TEXT("IPRedirectTool:"), MB_OK);
	return;
}

void CR3ToolDlg::OnBnClickedButtonUnloadDriver() { //关闭驱动
	if (bDriverIsLoad == FALSE) {
		MessageBox(TEXT("因为驱动服务没有启动,所以驱动服务关闭失败"), TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}
	if (hServer == NULL) {
		MessageBox(TEXT("无服务句柄!关闭失败!"), TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}
	SERVICE_STATUS status = { 0 };
	if (ControlService(hServer, SERVICE_CONTROL_STOP, &status) == 0) {
		CString ErrorMsg = L"";
		ErrorMsg.Format(L"关闭失败!错误代码:%d", GetLastError());
		MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}

	bDriverIsLoad = FALSE;
	MessageBox(TEXT("关闭成功"), TEXT("IPRedirectTool:"), MB_OK);
	return;
}

void CR3ToolDlg::OnBnClickedButtonUninstallDriver() { //卸载驱动
	if (DeleteService(hServer) == 0) {
		CString ErrorMsg = L"";
		ErrorMsg.Format(L"卸载失败!错误代码:%d", GetLastError());
		MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
		return;
	}
	if (hServer != NULL) {
		CloseServiceHandle(hServer);
	}
	bDriverIsInstall = FALSE;
	hServer = NULL;
	MessageBox(TEXT("卸载成功"), TEXT("IPRedirectTool:"), MB_OK);
}


DWORD CR3ToolDlg::IP2DwordData(CString cstrIpAddress){
	CString cstrTemp = L"";
	DWORD dwRet = 0;
	BYTE* pdwRet = (BYTE*) &dwRet;
	BYTE bIpByte = 0;
	
	AfxExtractSubString(cstrTemp, cstrIpAddress, 0, '.');
	bIpByte = _wtoi(cstrTemp);
	memset(pdwRet + 3, bIpByte, 1);

	AfxExtractSubString(cstrTemp, cstrIpAddress, 1, '.');
	bIpByte = _wtoi(cstrTemp);
	memset(pdwRet + 2, bIpByte, 1);

	AfxExtractSubString(cstrTemp, cstrIpAddress, 2, '.');
	bIpByte = _wtoi(cstrTemp);
	memset(pdwRet + 1, bIpByte, 1);

	AfxExtractSubString(cstrTemp, cstrIpAddress, 3, '.');
	bIpByte = _wtoi(cstrTemp);
	memset(pdwRet + 0, bIpByte, 1);

	return dwRet;
}

void CR3ToolDlg::OnBnClickedButtonRedirectSwitch(){

	CString cstrButtonName;
	GetDlgItemTextW(IDC_BUTTON_REDIRECT_SWITCH, cstrButtonName);
	if (lstrcmpW(cstrButtonName, L"Start") == 0) {
		// 开始重定向
		CString cstrLocalIp = L"";
		CString cstrTargetIp = L"";
		CString cstrTargetPort = L"";
		CString cstrLocalPort = L"";
		GetDlgItemTextW(IDC_EDIT_LOCAL_IP, cstrLocalIp);
		GetDlgItemTextW(IDC_EDIT_TARGET_IP, cstrTargetIp);
		GetDlgItemTextW(IDC_EDIT_TARGET_PORT, cstrTargetPort);
		GetDlgItemTextW(IDC_EDIT_LOCAL_PORT, cstrLocalPort);
		if ((lstrcmpW(cstrLocalIp, L"") == 0) || (lstrcmpW(cstrTargetIp, L"") == 0) || (lstrcmpW(cstrTargetPort, L"") == 0) || (lstrcmpW(cstrLocalPort, L"") == 0)) {
			MessageBox(TEXT("请先输入重定向信息"), TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}

		HANDLE hDevice = CreateFileW(SYMBOL_NAME, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDevice == INVALID_HANDLE_VALUE) {
			CString ErrorMsg = L"";
			ErrorMsg.Format(L"打开驱动设备失败!错误代码:%d", GetLastError());
			MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}

		//初始化结构体
		UpdateRedirect* pUpdateRedirect = new UpdateRedirect();



		pUpdateRedirect->dwUserLocalIPAddress = IP2DwordData(cstrLocalIp);
		pUpdateRedirect->dwUserTargetIPAddress = IP2DwordData(cstrTargetIp);
		pUpdateRedirect->wdUserLocalPort = GetDlgItemInt(IDC_EDIT_LOCAL_PORT, 0, 0);
		pUpdateRedirect->wdUserTargetPort = GetDlgItemInt(IDC_EDIT_TARGET_PORT, 0, 0);

		DWORD dwResult = 0;

		DWORD dwNumberOfBytesToWrite = 0;

		if (DeviceIoControl(hDevice, IOCTL_UPDATE_REDIRECT, pUpdateRedirect, sizeof(UpdateRedirect), &dwResult, sizeof(DWORD), &dwNumberOfBytesToWrite, NULL) == FALSE) {
			CString ErrorMsg = L"";
			ErrorMsg.Format(L"向驱动设备发送指令失败!错误代码:%d", GetLastError());
			MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}
		CloseHandle(hDevice);
		SetDlgItemTextW(IDC_BUTTON_REDIRECT_SWITCH, L"Stop");
		return;
	}
	if (lstrcmpW(cstrButtonName, L"Stop") == 0) {
		// 终止重定向

		HANDLE hDevice = CreateFileW(SYMBOL_NAME, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDevice == INVALID_HANDLE_VALUE) {
			CString ErrorMsg = L"";
			ErrorMsg.Format(L"打开驱动设备失败!错误代码:%d", GetLastError());
			MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}

		//初始化结构体
		UpdateRedirect* pUpdateRedirect = new UpdateRedirect();
		pUpdateRedirect->dwUserLocalIPAddress = 0;
		pUpdateRedirect->dwUserTargetIPAddress = 0;
		pUpdateRedirect->wdUserLocalPort = 0;
		pUpdateRedirect->wdUserTargetPort = 0;

		DWORD dwResult = 0;

		DWORD dwNumberOfBytesToWrite = 0;

		if (DeviceIoControl(hDevice, IOCTL_UPDATE_REDIRECT, pUpdateRedirect, sizeof(UpdateRedirect), &dwResult, sizeof(DWORD), &dwNumberOfBytesToWrite, NULL) == FALSE) {
			CString ErrorMsg = L"";
			ErrorMsg.Format(L"向驱动设备发送指令失败!错误代码:%d", GetLastError());
			MessageBox(ErrorMsg, TEXT("IPRedirectTool:"), MB_ICONERROR);
			return;
		}
		CloseHandle(hDevice);
		SetDlgItemTextW(IDC_BUTTON_REDIRECT_SWITCH, L"Start");
		return;
	}
}


int CR3ToolDlg::OnCreate(LPCREATESTRUCT lpCreateStruct){
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	MessageBox(TEXT("仅适用于64位环境下的Windows 10及Windows 11系统。\nBy LingMo"), TEXT("IPRedirectTool:"), MB_OK);

	return 0;
}

void CR3ToolDlg::OnDestroy(){

	CDialogEx::OnDestroy();
	OnBnClickedButtonUnloadDriver();
	OnBnClickedButtonUninstallDriver();
}
