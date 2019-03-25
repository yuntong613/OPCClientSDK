
// DemoDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Demo.h"
#include "DemoDlg.h"
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


// CDemoDlg 对话框



CDemoDlg::CDemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DEMO_DIALOG, pParent)
	, m_strRemote(_T(""))
	, m_strGroupName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADDR, m_strRemote);
	DDX_Text(pDX, IDC_EDIT_GROUP, m_strGroupName);
	DDX_Control(pDX, IDC_LIST_PROGID, m_lstProgIDs);
	DDX_Control(pDX, IDC_LIST2, m_strInfos);
	DDX_Control(pDX, IDC_LIST_ITEMS, m_lstItems);
	DDX_Control(pDX, IDC_LIST_ITEMS_VALUE, m_lstValues);
}

BEGIN_MESSAGE_MAP(CDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_GETLIST, &CDemoDlg::OnBnClickedButtonGetlist)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CDemoDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CDemoDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_ADD_GROUP, &CDemoDlg::OnBnClickedButtonAddGroup)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_GROUP, &CDemoDlg::OnBnClickedButtonRemoveGroup)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDemoDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_ADD_ITEMS, &CDemoDlg::OnBnClickedButtonAddItems)
END_MESSAGE_MAP()


// CDemoDlg 消息处理程序

BOOL CDemoDlg::OnInitDialog()
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

	m_lstValues.InsertColumn(0, "ItemID", HDF_LEFT, 150);
	m_lstValues.InsertColumn(1, "Value", HDF_LEFT,150);

	m_pSdk = OpcClientSDK::CreateOPCSDK();
	if (m_pSdk)
		m_pSdk->Initialize();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDemoDlg::OnPaint()
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDemoDlg::OnBnClickedButtonGetlist()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	m_lstProgIDs.ResetContent();
	if (m_pSdk)
	{
		std::vector<std::string> lst;
		if (m_pSdk->GetCLSIDList((LPCTSTR)m_strRemote, lst))
		{
			for (size_t i = 0; i < lst.size(); i++)
			{
				m_lstProgIDs.AddString(lst[i].c_str());
			}
		}
		else {
			std::string error;
			HRESULT result;
			CString strText;
			int nIdex = m_strInfos.AddString(error.c_str());
			m_strInfos.SetCaretIndex(nIdex);
		}

	}
}


void CDemoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	
	if (m_pSdk)
	{
		m_pSdk->Uninitialize();
		OpcClientSDK::DestroyOPCSDK(m_pSdk);
	}
}


CString GetOpcValue(VARIANT vtValue)
{
	CString strValue;
	switch (vtValue.vt)
	{
	case VT_BOOL:
		strValue.Format("%d", vtValue.boolVal ? 1 : 0);
		break;
	case VT_UI1:
		strValue.Format("%u", vtValue.bVal);
		break;
	case VT_I1:
		strValue.Format("%d", vtValue.cVal);
		break;
	case VT_UI2:
		strValue.Format("%u", vtValue.uiVal);
		break;
	case VT_I2:
		strValue.Format("%d", vtValue.iVal);
		break;
	case VT_UI4:
		strValue.Format("%u", vtValue.ulVal);
		break;
	case VT_I4:
		strValue.Format("%d", vtValue.lVal);
		break;
	case VT_R4:
		strValue.Format("%.2f", vtValue.fltVal);
		break;
	case VT_R8:
		strValue.Format("%.2f", vtValue.dblVal);
		break;
	case VT_BSTR:
		strValue = vtValue.bstrVal;
		break;
	default:
		strValue = "unknowm";
		break;
	}
	return strValue;
}

void MyOPCValueEventCallBack(char* groupName, char* strNodeName, VARIANT& vValue, int nQuailty, void* pUser)
{
	CDemoDlg* pDlg = (CDemoDlg*)pUser;
	if (pDlg && pDlg->GetSafeHwnd())
	{
		CString strText;
		strText.Format("Group %s, Item %s, Value %s", groupName, strNodeName, GetOpcValue(vValue));
		pDlg->AddLog(strText);
		pDlg->UpdateValue(strNodeName, GetOpcValue(vValue));
	}
}

void CDemoDlg::AddLog(const char* szText)
{
	m_strInfos.AddString(szText);
}

std::string ErrorMessage(HRESULT hr)
{
	void* pMsgBuf = NULL;

	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&pMsgBuf,
		0,
		NULL);


	char buff[2048] = { 0 };
	sprintf_s(buff, "ErrorCode %08x,Cause %s", hr, (LPTSTR)pMsgBuf);
	// Free the buffer.
	LocalFree(pMsgBuf);

	std::string eText = buff;
	return eText;
}

void CDemoDlg::OnBnClickedButtonConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_pSdk)
	{
		OPCException ex;
		CString strProgID;
		m_lstProgIDs.GetText(m_lstProgIDs.GetCurSel(), strProgID);
		if (!m_pSdk->ConnectServer(m_strRemote, strProgID,&ex))
		{
			std::string errortext;
			HRESULT code = 0;
	
			AddLog(ex.ErrorMessage().c_str());
		}
		m_pSdk->SetValueReport(MyOPCValueEventCallBack, this);
	}
}


void CDemoDlg::OnBnClickedButtonDisconnect()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_pSdk)
	{
		CString strProgID;
		m_lstProgIDs.GetText(m_lstProgIDs.GetCurSel(), strProgID);
		m_pSdk->DisConnectServer(m_strRemote, strProgID);
	}
}


void CDemoDlg::OnBnClickedButtonAddGroup()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_pSdk)
	{
		DWORD dwRate = 0;
		m_pSdk->AddGroup(m_strGroupName, dwRate);
	}
}


void CDemoDlg::OnBnClickedButtonRemoveGroup()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_pSdk)
	{
		m_pSdk->RemoveGroup(m_strGroupName);
	}
}


void CDemoDlg::OnBnClickedButtonBrowse()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_pSdk)
	{
		std::vector<std::string> items;
		m_pSdk->GetItemsList(items);
		for (size_t i = 0; i < items.size(); i++)
		{
			m_lstItems.AddString(items[i].c_str());
		}
	}
}
void CDemoDlg::UpdateValue(CString strItemId, CString strValue)
{
	LVFINDINFO it;
	it.psz = (LPCTSTR)strItemId;
	it.flags = LVFI_STRING;
	int nIndex = m_lstValues.FindItem(&it);
	if (nIndex!=-1)
	{
		m_lstValues.SetItemText(nIndex, 1, strValue);
	}else{
		m_lstValues.InsertItem(0, strItemId);
	}
}

void CDemoDlg::OnBnClickedButtonAddItems()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_pSdk)
	{
		CString strItemID;
		m_lstItems.GetText(m_lstItems.GetCurSel(), strItemID);

		std::vector<std::string> items;
		items.push_back((LPCTSTR)strItemID);
		if (m_pSdk->AddItems(m_strGroupName, items))
		{
			UpdateValue(strItemID, "");
		}
	}
}
