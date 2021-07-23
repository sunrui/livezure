#include "StdAfx.h"
#include "MainFrame.h"
#include "NavTree.h"

#include "service_api.h"
#include "db_account.h"
#include "db_provider.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MainFrame * pMainFrame;

	CPaintManagerUI::SetInstance(hInstance);
	CDuiString instpath = CPaintManagerUI::GetInstancePath();
	//	CPaintManagerUI::SetResourceZip(_T("360SafeRes.zip"));

	CoInitialize(NULL);

	pMainFrame = new MainFrame();
	pMainFrame->Create(NULL, _T("MainFrame UI"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	pMainFrame->CenterWindow();
	ShowWindow(*pMainFrame, SW_SHOW);

	CPaintManagerUI::MessageLoop();
	CoUninitialize();

	delete pMainFrame;

	return 0;
}

// navtree id -> layouttab id
#define NAVTREE_ID_BASIC			-1
#define NAVTREE_ID_BASIC_DEVICE		0
#define NAVTREE_ID_BASIC_ACCOUNT	1
#define NAVTREE_ID_BASIC_RECORD		2
#define NAVTREE_ID_BASIC_PROXY		3

MainFrame::MainFrame(void)
{
}

MainFrame::~MainFrame(void)
{
}

void MainFrame::InitWindow()
{
	InitNavTree();

	m_wndBasicAccount.SetPaintMagager(&m_PaintManager);
	m_wndBasicAccount.SetHwnd(m_hWnd);
	AddVirtualWnd(_T("basic_Account_wnd"),&m_wndBasicAccount);

	m_wndBasicDevice.SetPaintMagager(&m_PaintManager);
	m_wndBasicDevice.SetHwnd(m_hWnd);
	AddVirtualWnd(_T("basic_device_wnd"),&m_wndBasicDevice);

	m_wndBasicRecordSchedule.SetPaintMagager(&m_PaintManager);
	m_wndBasicRecordSchedule.SetHwnd(m_hWnd);
	AddVirtualWnd(_T("basic_record_schedule_wnd"),&m_wndBasicRecordSchedule);
}

CDuiString MainFrame::GetSkinFolder()
{
	return _T("skin");
}

CDuiString MainFrame::GetSkinFile()
{
	return _T("mainframe.xml");
}

LPCTSTR MainFrame::GetWindowClassName(void) const
{
	return _T("MainFrame UI");
}

LRESULT MainFrame::OnClose(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
	m_wndBasicAccount.DestroyAccountList();
	m_wndBasicDevice.DestroyDeviceList();

	EndDialog(m_hWnd, 0);
	PostQuitMessage(0);
	return 0;
}

LRESULT MainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);

	return 0;
}

class CSubContainerUI : public CContainerUI
{
public:
	CSubContainerUI(CPaintManagerUI *pm, LPCTSTR pstrClass) : m_pm(pm)
	{
		CDialogBuilder builder;
		CContainerUI *containerUI = NULL;

//		if(_tcscmp(pstrClass, _T("MainLogin")) == 0)
//		{
//			containerUI = static_cast<CContainerUI *>(builder.Create(_T("MainLogin.xml"), (UINT)0, NULL, m_pm));
//		}

		if (containerUI)
		{
			this->Add(containerUI);
		}
		else
		{
			this->RemoveAll();
		}
	}

private:
	CPaintManagerUI *m_pm;
};

CControlUI * MainFrame::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("NavTree")) == 0)
	{
		return new NavTreeUI();
	}

	return new CSubContainerUI(&m_PaintManager, pstrClass);
}

void MainFrame::Notify(TNotifyUI& msg)
{
	WindowImplBase::Notify(msg);
	
	if( msg.sType == _T("windowinit") )
	{
		OnPrepare();
	}

	if (msg.sType == _T("menu_create") || 
		msg.sType == _T("menu_modify") || 
		msg.sType == _T("menu_delete") || 
		msg.sType == _T("menu_detail"))
	{
		CListUI *pAccountList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("basic_account_list")));
		assert(pAccountList != NULL);
		if (pAccountList->IsVisible())
		{
			m_wndBasicAccount.Notify(msg);
			return;
		}
		
		CListUI *pDeviceList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("basic_device_list")));
		assert(pDeviceList != NULL);
		if (pDeviceList->IsVisible())
		{
			m_wndBasicDevice.Notify(msg);
			return;
		}
	}

	if (msg.sType == _T("itemclick"))
	{
		NavTreeUI * pNavTree = static_cast<NavTreeUI *>(this->m_PaintManager.FindControl(_T("NavTree")));
		if (pNavTree->GetItemIndex(msg.pSender) != -1)
		{
			if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0)
			{
				NavTreeUI::Node * node;
				CTabLayoutUI * pTabLayout;
				int cursel;

				node = (NavTreeUI::Node*)msg.pSender->GetTag();
				pTabLayout = static_cast<CTabLayoutUI *>(this->m_PaintManager.FindControl(_T("navtree_tab")));

				assert(node != NULL);
				assert(pTabLayout != NULL);

				cursel = pTabLayout->GetCurSel();
				if (cursel == node->data()._nLayoutID)
				{
					// click on the same item
					return;
				}

				if (node->data()._nLayoutID != -1)
				{
					pTabLayout->SelectItem(node->data()._nLayoutID);
				}

				{
					POINT pt = { 0 };
					::GetCursorPos(&pt);
					::ScreenToClient(m_PaintManager.GetPaintWindow(), &pt);
					pt.x -= msg.pSender->GetX();
					SIZE sz = pNavTree->GetExpanderSizeX(node);
					if( pt.x >= sz.cx && pt.x < sz.cy )
					{
						pNavTree->ExpandNode(node, !node->data()._expand);
					}
				}

				return;
			}
		}
	}
	else if (msg.sType == _T("itemactivate"))
	{
		{
			NavTreeUI * pNavTree = static_cast<NavTreeUI *>(m_PaintManager.FindControl(_T("NavTree")));
			if (pNavTree->GetItemIndex(msg.pSender) != -1)
			{
				if (_tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0) {
					NavTreeUI::Node* node = (NavTreeUI::Node*)msg.pSender->GetTag();
					pNavTree->ExpandNode(node, !node->data()._expand);
					return;
				}
			}
		}

		if (msg.sVirtualWnd == _T("basic_device_wnd"))
		{
			CListUI *pDeviceList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("basic_device_list")));
			assert(pDeviceList != NULL);
			int nSel = pDeviceList->GetCurSel();
			if (nSel < 0) return;

			m_wndBasicDevice.Notify(msg);
			return;
		}

		if (msg.sVirtualWnd == _T("basic_account_wnd"))
		{
			CListUI *pAccountList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("basic_account_list")));
			assert(pAccountList != NULL);
			int nSel = pAccountList->GetCurSel();
			if (nSel < 0) return;

			m_wndBasicAccount.Notify(msg);
			return;
		}
	}
}

void MainFrame::InitNavTree(void)
{
	NavTreeUI * pNavTree = static_cast<NavTreeUI *>(m_PaintManager.FindControl(_T("NavTree")));

	// 基本信息
	{
		NavTreeUI::Node * pNode = NULL;

		pNode = pNavTree->AddNode(NAVTREE_ID_BASIC, -1, _T("{x 3}{i navtree/tree_realmonitor.png 5 0} 基本信息"));
		pNavTree->AddNode(NAVTREE_ID_BASIC_DEVICE, 0, _T("{x 3}{i navtree/tree_realmonitor.png 5 2} 监控设备"), pNode);
		pNavTree->AddNode(NAVTREE_ID_BASIC_ACCOUNT, 1, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 注册用户"), pNode);
		pNavTree->AddNode(NAVTREE_ID_BASIC_RECORD, 2, _T("{x 3}{i navtree/tree_realmonitor.png 5 3} 录像计划"), pNode);
		pNavTree->AddNode(NAVTREE_ID_BASIC_PROXY, 3, _T("{x 3}{i navtree/tree_realmonitor.png 5 4} 负载服务器"), pNode);
	}

	// 回放查看
	{
		NavTreeUI::Node * pNode = NULL;

		pNode = pNavTree->AddNode(0, -1, _T("{x 3}{i navtree/tree_realmonitor.png 5 0} 回放查看"));
		pNavTree->AddNode(0, 0, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 监控录像"), pNode);
		pNavTree->AddNode(0, 1, _T("{x 3}{i navtree/tree_realmonitor.png 5 2} 系统日志"), pNode);
		pNavTree->AddNode(0, 2, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 报警联动"), pNode);
		//pNavTree->AddNode(_T("{x 3}{i navtree/tree_realmonitor.png 5 3} 存储空间"), pNode);
		//pNavTree->AddNode(_T("{x 3}{i navtree/tree_realmonitor.png 5 4} 硬件性能"), pNode);
	}

	// 系统工具
	{
		NavTreeUI::Node * pNode = NULL;

		pNode = pNavTree->AddNode(0, -1, _T("{x 3}{i navtree/tree_realmonitor.png 5 0} 系统工具"));
		pNavTree->AddNode(0, 0, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 诊断工具"), pNode);
		//pNavTree->AddNode(_T("{x 3}{i navtree/tree_realmonitor.png 5 1} 看门狗"), pNode);
		pNavTree->AddNode(0, 1, _T("{x 3}{i navtree/tree_realmonitor.png 5 3} 重启服务器"), pNode);
		pNavTree->AddNode(0, 2, _T("{x 3}{i navtree/tree_realmonitor.png 5 2} 恢复出厂设置"), pNode);
	}

	// 帮助与支持
	{
		NavTreeUI::Node * pNode = NULL;

		pNode = pNavTree->AddNode(0, -1, _T("{x 3}{i navtree/tree_realmonitor.png 5 0} 帮助与支持"));
		pNavTree->AddNode(0, 0, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 技术支持"), pNode);
		pNavTree->AddNode(0, 1, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 问题反馈"), pNode);
		pNavTree->AddNode(0, 2, _T("{x 3}{i navtree/tree_realmonitor.png 5 1} 关于 Livezure"), pNode);
	}
}

BOOL MainFrame::InitDb(void)
{
	const char * ip = NULL;
	int port = 0;
	const char * user = NULL;
	const char * pwd = NULL;
	const char * db = NULL;

	db = "d:/Develop/livezure_default.db";
	db_init(ip, port, user, pwd, db);

	{
		char err[65];
		int r;
		r = db_test_connect(err);
		if (!r)
		{
			MessageBox(m_hWnd, _T("init db failed."), NULL, MB_ICONWARNING);
			return FALSE;
		}
	}

	db_account_ensure_table_exist();

	return TRUE;
}

void MainFrame::OnPrepare()
{
	//MainLogin * login;
	//UINT uRet;

	//login = new MainLogin();
	//login->Create(m_hWnd, _T("MainLogin UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	//login->CenterWindow();
	//uRet = login->ShowModal();
	//if (uRet != IDYES)
	//{
	//	Close();
	//	return;
	//}

	InitDb();
	m_wndBasicAccount.InitAccountList();
	m_wndBasicDevice.InitDeviceList();
	m_wndBasicRecordSchedule.InitRecordScheduleList();
}
