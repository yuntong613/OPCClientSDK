
// DemoDlg.h: 头文件
//

#pragma once

#include "OpcClientSDK.h"

// CDemoDlg 对话框
class CDemoDlg : public CDialogEx
{
// 构造
public:
	CDemoDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	OpcClientSDK* m_pSdk;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strRemote;
	CString m_strGroupName;
	afx_msg void OnBnClickedButtonGetlist();
	afx_msg void OnDestroy();
	CListBox m_lstProgIDs;
	CListBox m_strInfos;
};
