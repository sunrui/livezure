#include "StdAfx.h"
#include "WndBasicDevice.h"

#include "db_account.h"
#include "xmalloc.h"
#include "MenuWnd.h"
#include "WndBasicDeviceAdd.h"
#include "WndBasicDeviceModify.h"
#include "WndBasicDeviceDetail.h"
#include "WndBasicDeviceDelete.h"

DUI_BEGIN_MESSAGE_MAP(WndBasicDevice, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_MENU, OnMenu)
DUI_END_MESSAGE_MAP()

WndBasicDevice::WndBasicDevice(void)
{
	m_pContextMenu = NULL;
}

WndBasicDevice::~WndBasicDevice(void)
{
}

void WndBasicDevice::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	assert(pPaintMgr != NULL);
	m_pPaintManager = pPaintMgr;
}

void WndBasicDevice::SetHwnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

void WndBasicDevice::OnClick(TNotifyUI& msg)
{

}

void WndBasicDevice::OnSelectChanged(TNotifyUI &msg)
{

}

void WndBasicDevice::OnItemClick(TNotifyUI &msg)
{
		   //( msg.sType == _T("menu_Delete") ) {
			  // CListUI* pList = static_cast<CListUI*>(msg.pSender);
			  // int nSel = pList->GetCurSel();
			  // if( nSel < 0 ) return;
			  // pList->RemoveAt(nSel);
			  // domain.erase(domain.begin() + nSel);
			  // desc.erase(desc.begin() + nSel);   
		   //}
}

void WndBasicDevice::InitDeviceList()
{
	int count;
	bool r;
	int i;

	// 为测试添加用户
	db_account_device_t ac_dec;
	for (i = 0; i < 10; i++)
	{
		db_device_basic_t dec;

		sprintf(dec.uuid, "uuid_%d", i); /* unique id of device */
		sprintf(dec.plugin_name, "plusin_%d", i); /* registered plugin name */
		sprintf(dec.ip, "192.168.1.%d", i); /* device ip */
		dec.port = i; /* device port */
		sprintf(dec.user, "user_%d", i); /* device login user */
		sprintf(dec.pwd, "pwd_%d", i); /* device login pwd */
		dec.channel = i; /* open channel number */
		sprintf(dec.seg_folder, "seg_folder_%d", i); /* saved segmenter folder */
		dec.seg_in_count = 3; /* how many segmenter files in one seg_file */
		dec.seg_per_sec = 10; /* segmenter per-second */
		//char created_time[64]; /* automate generate created time */

		ac_dec.basic = dec;
		sprintf(ac_dec.comment, "commen_%d", i);
		ac_dec.is_top_level = 1;
		sprintf(ac_dec.nickname, "device_nick_%d", i);
		sprintf(ac_dec.ref_parent_uuid, "parent_uuid_%d", i);

		db_account_device_add("root", &ac_dec, NULL);
	}
	// 为测试添加用户 END

	db_account_device_t ** pac_dec;
	r = db_account_device_get("root", NULL, &pac_dec, &count);
	for (i = 0; i < count; i++)
	{
		db_account_device_t * clone;

		clone = (db_account_device_t *)xcalloc(1, sizeof(db_account_device_t));
		memcpy(clone, pac_dec[i], sizeof(db_account_device_t));
		DeviceInsertToUI(clone, FALSE);
	}

	for (i = 0; i < count; i++)
		DeviceSetOnline(pac_dec[i]->basic.uuid, i % 2);

	db_account_device_free(&pac_dec, count);
}

void WndBasicDevice::DestroyDeviceList()
{
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int count = pDeviceList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(index);
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

void WndBasicDevice::OnMenu(TNotifyUI& msg)
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

void WndBasicDevice::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("menu_create"))
	{
		OnDeviceCreate();
	}
	else if (msg.sType == _T("menu_modify") || 
		msg.sType == _T("itemactivate"))
	{
		OnDeviceMofidy();
	}
	else if (msg.sType == _T("menu_delete"))
	{
		OnDeviceDelete();
	}
	else if (msg.sType == _T("menu_detail"))
	{
		OnDeviceDetail();
	}
}

void WndBasicDevice::DeviceInsertToUI(db_account_device_t * device, BOOL bIsOnline)
{
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	CListContainerElementUI * new_node = new CListContainerElementUI;
	new_node->ApplyAttributeList(_T("height=\"30\""));
	new_node->SetTag((UINT_PTR)device);

	CHorizontalLayoutUI * new_h_lay = new CHorizontalLayoutUI;

	// 运行状态
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

	// 设备昵称
	{
		CLabelUI * nickname = new CLabelUI;
		nickname->ApplyAttributeList(_T("name=\"nickname\" width=\"120\""));

		TCHAR * pstr_nickname;
		pstr_nickname = mbs_to_wcs(device->nickname);
		nickname->SetText(pstr_nickname);
		free(pstr_nickname);

		new_h_lay->Add(nickname);
	}

	// 设备类型
	{
		CLabelUI * plugin_name = new CLabelUI;
		plugin_name->ApplyAttributeList(_T("name=\"plugin_name\" width=\"120\""));

		TCHAR * pstr_plugin_name;
		pstr_plugin_name = mbs_to_wcs(device->basic.plugin_name);
		plugin_name->SetText(pstr_plugin_name);
		free(pstr_plugin_name);

		new_h_lay->Add(plugin_name);
	}

	// IP 地址
	{
		CLabelUI * ipaddr = new CLabelUI;
		ipaddr->ApplyAttributeList(_T("name=\"ipaddr\" width=\"110\""));

		TCHAR * pstr_ipaddr;
		pstr_ipaddr = mbs_to_wcs(device->basic.ip);
		ipaddr->SetText(pstr_ipaddr);
		free(pstr_ipaddr);

		new_h_lay->Add(ipaddr);
	}

	// 端口
	{
		CLabelUI * port = new CLabelUI;
		port->ApplyAttributeList(_T("name=\"port\" width=\"50\""));

		TCHAR pstr_port[32];
		wsprintf(pstr_port, _T("%d"), device->basic.port);
		port->SetText(pstr_port);

		new_h_lay->Add(port);
	}

	// 通道号
	{
		CLabelUI * chanel = new CLabelUI;
		chanel->ApplyAttributeList(_T("name=\"channel\" width=\"50\""));

		TCHAR pstr_chanel[32];
		wsprintf(pstr_chanel, _T("%d"), device->basic.channel);
		chanel->SetText(pstr_chanel);

		new_h_lay->Add(chanel);
	}

	// 工作目录
	{
		CLabelUI * workingfolder = new CLabelUI;
		workingfolder->ApplyAttributeList(_T("name=\"working_folder\" width=\"100\""));

		TCHAR * pstr_tmp;
		pstr_tmp = mbs_to_wcs(device->basic.seg_folder);
		workingfolder->SetText(pstr_tmp);
		free(pstr_tmp);

		new_h_lay->Add(workingfolder);
	}

	// 创建时间
	{
		CLabelUI * create_time = new CLabelUI;
		create_time->ApplyAttributeList(_T("name=\"created_time\" width=\"150\""));

		TCHAR * pstr_tmp;
		pstr_tmp = mbs_to_wcs(device->basic.created_time);
		create_time->SetText(pstr_tmp);
		free(pstr_tmp);

		new_h_lay->Add(create_time);
	}

	// 备注
	{
		CLabelUI * comment = new CLabelUI;
		comment->ApplyAttributeList(_T("name=\"comment\" width=\"200\""));

		TCHAR * pstr_tmp;
		pstr_tmp = mbs_to_wcs(device->comment);
		comment->SetText(pstr_tmp);
		free(pstr_tmp);

		new_h_lay->Add(comment);
	}

	// 操作按钮
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
	pDeviceList->Add(new_node);
}

void WndBasicDevice::DeviceSetOnline(const char * uuid, BOOL bIsOnline)
{
	assert(uuid != NULL);

	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int count = pDeviceList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(index);
		db_account_device_t * device;

		device = (db_account_device_t *)item->GetTag();
		assert(device != NULL);
		
		if (strcmp(device->basic.uuid, uuid) != 0)
		{
			continue;
		}

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

void WndBasicDevice::OnDeviceMofidy()
{
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int nSel = pDeviceList->GetCurSel();
	if (nSel < 0) return;

	CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(nSel);
	db_account_device_t * device;

	device = (db_account_device_t *)item->GetTag();
	assert(device != NULL);

	WndBasicDeviceModify * modify;
	modify = new WndBasicDeviceModify(this, device);
	modify->Create(m_hWnd, _T("WndBasicDeviceAdd UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	modify->CenterWindow();
	modify->ShowModal();
}

void WndBasicDevice::OnDeviceDetail()
{
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int nSel = pDeviceList->GetCurSel();
	if (nSel < 0) return;

	CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(nSel);
	db_account_device_t * device;

	device = (db_account_device_t *)item->GetTag();
	assert(device != NULL);

	WndBasicDeviceDetail * detail;
	detail = new WndBasicDeviceDetail(this, device);
	detail->Create(m_hWnd, _T("WndBasicDeviceDetail UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	detail->CenterWindow();
	detail->ShowModal();
}

void WndBasicDevice::DeviceDelete(const char * uuid)
{
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int count = pDeviceList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(index);
		db_account_device_t * device;

		device = (db_account_device_t *)item->GetTag();
		assert(device != NULL);

		if (strcmp(device->basic.uuid, uuid) == 0)
		{
			db_account_device_del("root", uuid);
			pDeviceList->RemoveAt(index);
			xfree(device);
			break;
		}
	}
}

void WndBasicDevice::OnDeviceDelete()
{	
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int nSel = pDeviceList->GetCurSel();
	if (nSel < 0) return;

	CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(nSel);
	db_account_device_t * device;

	device = (db_account_device_t *)item->GetTag();
	assert(device != NULL);

	{
		WndBasicDeviceDelete * del;
		UINT nRet;

		del = new WndBasicDeviceDelete(this, device);
		del->Create(m_hWnd, _T("WndBasicDeviceDelete UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
		del->CenterWindow();
		nRet = del->ShowModal();
		if (nRet == IDYES)
		{
			DeviceDelete(device->basic.uuid);
		}
	}
}

void WndBasicDevice::OnDeviceCreate()
{
	WndBasicDeviceAdd * add;
	add = new WndBasicDeviceAdd(this);
	add->Create(m_hWnd, _T("WndBasicDeviceAdd UI"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
	add->CenterWindow();
	add->ShowModal();
}

void WndBasicDevice::DeviceUpdate(const char * uuid, db_account_device_t * device)
{
	CListUI *pDeviceList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_device_list")));
	assert(pDeviceList != NULL);

	int count = pDeviceList->GetCount();
	for (int index = 0; index < count; index++)
	{
		CListContainerElementUI * item = (CListContainerElementUI *)pDeviceList->GetItemAt(index);
		db_account_device_t * device;

		device = (db_account_device_t *)item->GetTag();
		assert(device != NULL);

		if (strcmp(device->basic.uuid, uuid) == 0)
		{			
			{
				db_account_device_t * itemdev = (db_account_device_t *)item->GetTag();
				assert(itemdev != NULL);

				strncpy(itemdev->nickname, device->nickname, sizeof(itemdev->nickname) - 1);
				strncpy(itemdev->comment, device->comment, sizeof(itemdev->comment) - 1);
				strncpy(itemdev->basic.user, device->basic.user, sizeof(itemdev->basic.user) - 1);
				strncpy(itemdev->basic.pwd, device->basic.pwd, sizeof(itemdev->basic.pwd) - 1);
				strncpy(itemdev->basic.seg_folder, device->basic.seg_folder, sizeof(itemdev->basic.seg_folder) - 1);
			}

			{
				CLabelUI * nickname = (CLabelUI *)item->FindSubControl(_T("nickname"));
				TCHAR * w_nickname;
				w_nickname = mbs_to_wcs(device->nickname);
				nickname->SetText(w_nickname);
				free(w_nickname);
			}

			{
				CLabelUI * comment = (CLabelUI *)item->FindSubControl(_T("comment"));
				TCHAR * w_comment;
				w_comment = mbs_to_wcs(device->comment);
				comment->SetText(w_comment);
				free(w_comment);
			}

			{
				CLabelUI * working_folder = (CLabelUI *)item->FindSubControl(_T("working_folder"));
				TCHAR * w_working_folder;
				w_working_folder = mbs_to_wcs(device->basic.seg_folder);
				working_folder->SetText(w_working_folder);
				free(w_working_folder);
			}

			break;
		}
	}
}
