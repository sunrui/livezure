#include "StdAfx.h"
#include "WndBasicAccount.h"

#include "db_account.h"
#include "xmalloc.h"
#include "MenuWnd.h"
#include "WndBasicAccountAdd.h"
#include "WndBasicAccountModify.h"
#include "WndBasicAccountDelete.h"
#include "WndBasicAccountDetail.h"

DUI_BEGIN_MESSAGE_MAP(WndBasicAccount, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_MENU, OnMenu)
DUI_END_MESSAGE_MAP()

WndBasicAccount::WndBasicAccount(void)
{
	m_pContextMenu = NULL;
}

WndBasicAccount::~WndBasicAccount(void)
{
}

void WndBasicAccount::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	assert(pPaintMgr != NULL);
	m_pPaintManager = pPaintMgr;
}

void WndBasicAccount::SetHwnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

void WndBasicAccount::OnClick(TNotifyUI& msg)
{

}

void WndBasicAccount::OnSelectChanged(TNotifyUI &msg)
{

}

void WndBasicAccount::OnItemClick(TNotifyUI &msg)
{

}

void WndBasicAccount::OnMenu(TNotifyUI& msg)
{
	POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
	::ClientToScreen(m_hWnd, &pt);

	if (m_pContextMenu == NULL)
	{
		m_pContextMenu = new CMenuWnd();
		m_pContextMenu->Init(msg.pSender, pt);
	}

	m_pContextMenu->OnShow(pt);
}

void WndBasicAccount::InitAccountList()
{
	db_account_basic_t ** child;
	int count;
	bool r;
	int i;

	// 为测试添加用户
	db_account_basic_t acc;

	sprintf(acc.uuid, "root");
	sprintf(acc.nickname, "nickname_root");
	sprintf(acc.comment, "comment_root");
	sprintf(acc.pwd, "root");
	db_account_admin_add(&acc);

	for (i = 0; i < 10; i++)
	{
		sprintf(acc.uuid, "uuid_%d", i);
		sprintf(acc.nickname, "nickname_%d", i);
		sprintf(acc.comment, "comment_%d", i);
		sprintf(acc.pwd, "123");

		db_account_child_add("root", &acc, NULL);
	}

	// 为测试添加用户 END
	r = db_account_list_child_get("root", NULL, &child, &count);
	for (i = 0; i < count; i++)
	{
		db_account_basic_t * clone;

		clone = (db_account_basic_t *)xcalloc(1, sizeof(db_account_basic_t));
		memcpy(clone, child[i], sizeof(db_account_basic_t));
		AccountInsertToUI(clone, FALSE);
	}

	for (i = 0; i < count; i++)
		AccountSetOnline(child[i]->uuid, i % 2);

	db_account_list_free(&child, count);
}

void WndBasicAccount::DestroyAccountList()
{
	CListUI *pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int count = pAccountList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(index);
		db_account_device_t * device;

		device = (db_account_device_t *)item->GetTag();
		assert(device != NULL);

		xfree(device);
	}

	if (m_pContextMenu != NULL)
	{
		m_pContextMenu->Close();
		m_pContextMenu = NULL;
	}
}

void WndBasicAccount::Notify(TNotifyUI& msg)
{
	if ( msg.sType == _T("menu_create"))
	{
		OnAccountCreate();
	}
	else if ( msg.sType == _T("menu_modify") || 
		msg.sType == _T("itemactivate"))
	{
		OnAccountModify();
	}
	else if ( msg.sType == _T("menu_delete"))
	{
		OnAccountDelete();
	}
	else if ( msg.sType == _T("menu_detail"))
	{
		OnAccountDetail();
	}
}

void WndBasicAccount::OnAccountCreate()
{
	WndBasicAccountAdd * add;
	add = new WndBasicAccountAdd(this);
	add->Create(m_hWnd, _T("WndBasicAccountAdd UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	add->CenterWindow();
	add->ShowModal();
}

void WndBasicAccount::AccountInsertToUI(db_account_basic_t * account, BOOL bIsOnline)
{
	CListUI *pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	CListContainerElementUI * new_node = new CListContainerElementUI;
	new_node->ApplyAttributeList(_T("height=\"30\""));
	new_node->SetTag((UINT_PTR)account);

	CHorizontalLayoutUI * new_h_lay = new CHorizontalLayoutUI;

	{
		CVerticalLayoutUI * item_v_lay = new CVerticalLayoutUI;
		item_v_lay->ApplyAttributeList(_T("width=\"60\""));
		CButtonUI * item_btn = new CButtonUI;
		TCHAR temp_attr[256];
		_stprintf(temp_attr, 
			_T("padding=\"23,8,0,0\" name=\"account_online_btn\" width=\"14\" height=\"14\" bkimage=\"%s\""),
			bIsOnline ? _T("listctrl/working_green.png") : _T("listctrl/working_red.png"));
		item_btn->ApplyAttributeList(temp_attr);
		item_v_lay->Add(item_btn);
		new_h_lay->Add(item_v_lay);
	}

	{
		CLabelUI * nickname = new CLabelUI;
		nickname->ApplyAttributeList(_T("name=\"nickname\" width=\"130\""));
		
		TCHAR * pstr_nickName;
		pstr_nickName = mbs_to_wcs(account->nickname);
		nickname->SetText(pstr_nickName);
		free(pstr_nickName);

		new_h_lay->Add(nickname);
	}

	{
		CLabelUI * uuid = new CLabelUI;
		uuid->ApplyAttributeList(_T("name=\"uuid\" width=\"130\""));

		TCHAR * pstr_uuid;
		pstr_uuid = mbs_to_wcs(account->uuid);
		uuid->SetText(pstr_uuid);
		free(pstr_uuid);

		new_h_lay->Add(uuid);
	}

	{
		CLabelUI * comment = new CLabelUI;
		comment->ApplyAttributeList(_T("name=\"comment\" width=\"165\""));

		TCHAR * pstr_comment;
		pstr_comment = mbs_to_wcs(account->comment);
		comment->SetText(pstr_comment);
		free(pstr_comment);
		new_h_lay->Add(comment);
	}

	{
		CLabelUI * created_time = new CLabelUI;
		created_time->ApplyAttributeList(_T("name=\"createdtime\" width=\"150\""));

		TCHAR * pstr_createdtime;
		pstr_createdtime = mbs_to_wcs(account->created_time);
		created_time->SetText(pstr_createdtime);
		free(pstr_createdtime);
		new_h_lay->Add(created_time);
	}
	
	//{
	//	CHorizontalLayoutUI * operation = new CHorizontalLayoutUI;
	//	for (int operation_id = 0; operation_id < 4; operation_id++)
	//	{
	//		struct operation_icon_st
	//		{
	//			TCHAR icon_operation[260];
	//			TCHAR icon_normal[260];
	//			TCHAR icon_hover[260];
	//			TCHAR icon_active[260];
	//		} operation_icon[] = 
	//		{
	//			{ _T("start"), _T("listctrl/working_start_normal.png"), _T("listctrl/wokring_start_hover.png"), _T("listctrl/working_start_active.png") },
	//			{ _T("pause"), _T("listctrl/working_pause_normal.png"), _T("listctrl/wokring_pause_hover.png"), _T("listctrl/working_pause_active.png") },
	//			{ _T("delete"), _T("listctrl/working_delete_normal.png"), _T("listctrl/wokring_delete_hover.png"), _T("listctrl/working_delete_active.png") },
	//			{ _T("search"), _T("listctrl/working_search_normal.png"), _T("listctrl/wokring_search_hover.png"), _T("listctrl/working_search_active.png") },
	//		};

	//		operation->ApplyAttributeList(
	//			_T("name=\"operation\" visiable=\"true\" width=\"160\""));
	//		CButtonUI * device_start = new CButtonUI;
	//		TCHAR temp_attr[256];
	//		_stprintf(temp_attr, 
	//			_T("padding=\"0,1,0,0\" name=\"%s\" width=\"28\" height=\"28\" ")
	//			_T("bkimage=\"%s\" ")
	//			_T("hotimage=\"%s\" ")
	//			_T("pushedimage=\"%s\""),
	//			operation_icon[operation_id].icon_operation,
	//			operation_icon[operation_id].icon_normal, operation_icon[operation_id].icon_hover, operation_icon[operation_id].icon_active);
	//		device_start->ApplyAttributeList(temp_attr);
	//		operation->Add(device_start);
	//	}
	//	new_h_lay->Add(operation);
	//}

	new_node->Add(new_h_lay);
	pAccountList->Add(new_node);
}

void WndBasicAccount::AccountSetOnline(const char * uuid, BOOL bIsOnline)
{
	assert(uuid != NULL);

	CListUI * pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int count = pAccountList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(index);
		CLabelUI * uuid_ctrl = (CLabelUI *)item->FindSubControl(_T("uuid"));
		
		if (uuid_ctrl == NULL) continue;

		TCHAR * w_uuid = mbs_to_wcs(uuid);
		LPCTSTR w_ctrl_uuid = uuid_ctrl->GetText().GetData();
		BOOL is_cur_uuid = _tcsicmp(w_ctrl_uuid, w_uuid) == 0;
		free(w_uuid);

		if (is_cur_uuid)
		{
			CButtonUI * online_btn = (CButtonUI *)item->FindSubControl(_T("account_online_btn"));

			TCHAR temp_attr[256];
			_stprintf(temp_attr, 
				_T("padding=\"23,8,0,0\" name=\"account_online_btn\" width=\"14\" height=\"14\" bkimage=\"%s\""),
				bIsOnline ? _T("listctrl/working_green.png") : _T("listctrl/working_red.png"));
			online_btn->ApplyAttributeList(temp_attr);
			break;
		}
	}
}

void WndBasicAccount::OnAccountModify()
{
	CListUI *pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int nSel = pAccountList->GetCurSel();
	if (nSel < 0) return;

	CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(nSel);
	db_account_basic_t * account;

	account = (db_account_basic_t *)item->GetTag();
	assert(account != NULL);

	WndBasicAccountModify * modify;
	modify = new WndBasicAccountModify(this, account);
	modify->Create(m_hWnd, _T("WndBasicAccountModify UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	modify->CenterWindow();
	modify->ShowModal();
}

void WndBasicAccount::AccountUpdate(const char * uuid, db_account_basic_t * account)
{
	assert(uuid != NULL);

	CListUI * pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int count = pAccountList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(index);
		CLabelUI * uuid_ctrl = (CLabelUI *)item->FindSubControl(_T("uuid"));

		if (uuid_ctrl == NULL) continue;

		TCHAR * w_uuid = mbs_to_wcs(uuid);
		LPCTSTR w_ctrl_uuid = uuid_ctrl->GetText().GetData();
		BOOL is_cur_uuid = _tcsicmp(w_ctrl_uuid, w_uuid) == 0;
		free(w_uuid);

		if (is_cur_uuid)
		{
			{
				db_account_basic_t * def_acc = (db_account_basic_t *)item->GetTag();
				assert(def_acc != NULL);

				strncpy(def_acc->nickname, account->nickname, sizeof(def_acc->nickname) - 1);
				strncpy(def_acc->comment, account->comment, sizeof(def_acc->comment) - 1);
			}

			{
				CLabelUI * nickname = (CLabelUI *)item->FindSubControl(_T("nickname"));
				TCHAR * w_nickname;
				w_nickname = mbs_to_wcs(account->nickname);
				nickname->SetText(w_nickname);
				free(w_nickname);
			}
			
			{
				CLabelUI * comment = (CLabelUI *)item->FindSubControl(_T("comment"));
				TCHAR * w_comment;
				w_comment = mbs_to_wcs(account->comment);
				comment->SetText(w_comment);
				free(w_comment);
			}

			break;
		}
	}
}

void WndBasicAccount::OnAccountDelete()
{
	CListUI *pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int nSel = pAccountList->GetCurSel();
	if (nSel < 0) return;

	CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(nSel);
	db_account_basic_t * account;

	account = (db_account_basic_t *)item->GetTag();
	assert(account != NULL);

	{
		WndBasicAccountDelete * del;
		UINT nRet;

		del = new WndBasicAccountDelete(this, account);
		del->Create(m_hWnd, _T("WndBasicAccountDelete UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
		del->CenterWindow();
		nRet = del->ShowModal();
		if (nRet == IDYES)
		{
			AccountDelete(account->uuid);
		}
	}
}

void WndBasicAccount::AccountDelete(const char * uuid)
{
	CListUI *pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int count = pAccountList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(index);
		db_account_basic_t * device;

		device = (db_account_basic_t *)item->GetTag();
		assert(device != NULL);

		if (strcmp(device->uuid, uuid) == 0)
		{
			db_account_child_del("root", uuid);
			pAccountList->RemoveAt(index);
			xfree(device);
			break;
		}
	}
}

void WndBasicAccount::OnAccountDetail()
{
	CListUI *pAccountList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_account_list")));
	assert(pAccountList != NULL);

	int nSel = pAccountList->GetCurSel();
	if (nSel < 0) return;

	CListContainerElementUI * item = (CListContainerElementUI *)pAccountList->GetItemAt(nSel);
	db_account_basic_t * acc;

	acc = (db_account_basic_t *)item->GetTag();
	assert(acc != NULL);

	WndBasicAccountDetail * account;
	account = new WndBasicAccountDetail(this, acc);
	account->Create(m_hWnd, _T("WndBasicAccountDetail UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	account->CenterWindow();
	account->ShowModal();
}
