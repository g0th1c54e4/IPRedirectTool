#pragma once
#include <winsvc.h>
#include "Driver.h"

// CR3ToolDlg 对话框
class CR3ToolDlg : public CDialogEx
{
// 构造
public:
	CR3ToolDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_R3TOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	SC_HANDLE hServer = 0;
	BOOL bDriverIsInstall = FALSE;
	BOOL bDriverIsLoad = FALSE;
	
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	DWORD IP2DwordData(CString cstrIpAddress); //将IP地址转换为DWORD数据
public:
	//afx_msg void OnBnClickedButtonLoadDriver();
	afx_msg void OnBnClickedButtonUnloadDriver();


	afx_msg void OnBnClickedButtonRedirectSwitch();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedButtonInstallDriver();
	afx_msg void OnBnClickedButtonUninstallDriver();
	afx_msg void OnBnClickedButtonLoadDriver();
	afx_msg void OnDestroy();
};
