#include "StdAfx.h"
#include "WndBasicRecordSchedule.h"

#include "xmalloc.h"

//void RecordScheduleInsertToUI(CPaintManagerUI * m_pPaintManager, db_RecordSchedule_basic_t * RecordSchedule, BOOL bIsOnline);
void RecordScheduleSetOnline(CPaintManagerUI * m_pPaintManager, const char * uuid, BOOL bIsOnline);

DUI_BEGIN_MESSAGE_MAP(WndBasicRecordSchedule, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
	DUI_END_MESSAGE_MAP()

	WndBasicRecordSchedule::WndBasicRecordSchedule(void)
{
}

WndBasicRecordSchedule::~WndBasicRecordSchedule(void)
{
}

void WndBasicRecordSchedule::SetPaintMagager(CPaintManagerUI* pPaintMgr)
{
	assert(pPaintMgr != NULL);
	m_pPaintManager = pPaintMgr;
}

void WndBasicRecordSchedule::SetHwnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

void WndBasicRecordSchedule::OnClick(TNotifyUI& msg)
{

}

void WndBasicRecordSchedule::OnSelectChanged(TNotifyUI &msg)
{

}

void WndBasicRecordSchedule::OnItemClick(TNotifyUI &msg)
{

}

void WndBasicRecordSchedule::InitRecordScheduleList()
{
	//db_RecordSchedule_basic_t ** child;
	//int count;
	//bool r;
	//int i;

	//// 为测试添加用户
	//db_RecordSchedule_basic_t acc;

	//sprintf(acc.uuid, "root");
	//sprintf(acc.nickname, "nickname_root");
	//sprintf(acc.comment, "comment_root");
	//sprintf(acc.pwd, "root");
	//db_RecordSchedule_admin_add(&acc);

	//for (i = 0; i < 10; i++)
	//{
	//	sprintf(acc.uuid, "uuid_%d", i);
	//	sprintf(acc.nickname, "nickname_%d", i);
	//	sprintf(acc.comment, "comment_%d", i);
	//	sprintf(acc.pwd, "123");

	//	db_RecordSchedule_child_add("root", &acc);
	//}

	//// 为测试添加用户 END
	//r = db_RecordSchedule_list_child_get("root", NULL, &child, &count);
	//for (i = 0; i < count; i++)
	//{
	//	db_RecordSchedule_basic_t * clone;

	//	clone = (db_RecordSchedule_basic_t *)xcalloc(1, sizeof(db_RecordSchedule_basic_t));
	//	memcpy(clone, child[i], sizeof(db_RecordSchedule_basic_t));
	//	RecordScheduleInsertToUI(m_pPaintManager, clone, FALSE);
	//}

	//for (i = 0; i < count; i++)
	//	RecordScheduleSetOnline(m_pPaintManager, child[i]->uuid, i % 2);

	//db_RecordSchedule_list_free(&child, count);
}

void WndBasicRecordSchedule::DestroyRecordScheduleList()
{
	//CListUI *pRecordScheduleList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_RecordSchedule_list")));
	//assert(pRecordScheduleList != NULL);

	//int count = pRecordScheduleList->GetCount();
	//for (int index = 0; index < count; index++)
	//{
	//	CListContainerElementUI * item = (CListContainerElementUI *)pRecordScheduleList->GetItemAt(index);
	//	db_RecordSchedule_device_t * device;

	//	device = (db_RecordSchedule_device_t *)item->GetTag();
	//	assert(device != NULL);

	//	xfree(device);
	//}
}
//
//void RecordScheduleInsertToUI(CPaintManagerUI * m_pPaintManager, db_RecordSchedule_basic_t * RecordSchedule, BOOL bIsOnline)
//{
//	CListUI *pRecordScheduleList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_RecordSchedule_list")));
//	assert(pRecordScheduleList != NULL);
//
//	CListContainerElementUI * new_node = new CListContainerElementUI;
//	new_node->ApplyAttributeList(_T("height=\"30\""));
//	new_node->SetTag((UINT_PTR)RecordSchedule);
//
//	CHorizontalLayoutUI * new_h_lay = new CHorizontalLayoutUI;
//
//	{
//		CVerticalLayoutUI * item_v_lay = new CVerticalLayoutUI;
//		item_v_lay->ApplyAttributeList(_T("width=\"60\""));
//		CButtonUI * item_btn = new CButtonUI;
//		TCHAR temp_attr[256];
//		_stprintf(temp_attr, 
//			_T("padding=\"23,8,0,0\" name=\"RecordSchedule_online_btn\" width=\"14\" height=\"14\" bkimage=\"%s\""),
//			bIsOnline ? _T("listctrl/working_green.png") : _T("listctrl/working_red.png"));
//		item_btn->ApplyAttributeList(temp_attr);
//		item_v_lay->Add(item_btn);
//		new_h_lay->Add(item_v_lay);
//	}
//
//	{
//		CLabelUI * nickname = new CLabelUI;
//		nickname->ApplyAttributeList(_T("name=\"nickname\" width=\"130\""));
//
//		TCHAR * pstr_nickName;
//		pstr_nickName = mbs_to_wcs(RecordSchedule->nickname);
//		nickname->SetText(pstr_nickName);
//		free(pstr_nickName);
//
//		new_h_lay->Add(nickname);
//	}
//
//	{
//		CLabelUI * uuid = new CLabelUI;
//		uuid->ApplyAttributeList(_T("name=\"uuid\" width=\"130\""));
//
//		TCHAR * pstr_uuid;
//		pstr_uuid = mbs_to_wcs(RecordSchedule->uuid);
//		uuid->SetText(pstr_uuid);
//		free(pstr_uuid);
//
//		new_h_lay->Add(uuid);
//	}
//
//	{
//		CLabelUI * comment = new CLabelUI;
//		comment->ApplyAttributeList(_T("name=\"comment\" width=\"165\""));
//
//		TCHAR * pstr_comment;
//		pstr_comment = mbs_to_wcs(RecordSchedule->comment);
//		comment->SetText(pstr_comment);
//		free(pstr_comment);
//		new_h_lay->Add(comment);
//	}
//
//	{
//		CLabelUI * created_time = new CLabelUI;
//		created_time->ApplyAttributeList(_T("name=\"createdtime\" width=\"150\""));
//
//		TCHAR * pstr_createdtime;
//		pstr_createdtime = mbs_to_wcs(RecordSchedule->created_time);
//		created_time->SetText(pstr_createdtime);
//		free(pstr_createdtime);
//		new_h_lay->Add(created_time);
//	}
//
//	{
//		CHorizontalLayoutUI * operation = new CHorizontalLayoutUI;
//		for (int operation_id = 0; operation_id < 4; operation_id++)
//		{
//			struct operation_icon_st
//			{
//				TCHAR icon_operation[260];
//				TCHAR icon_normal[260];
//				TCHAR icon_hover[260];
//				TCHAR icon_active[260];
//			} operation_icon[] = 
//			{
//				{ _T("start"), _T("listctrl/working_start_normal.png"), _T("listctrl/wokring_start_hover.png"), _T("listctrl/working_start_active.png") },
//				{ _T("pause"), _T("listctrl/working_pause_normal.png"), _T("listctrl/wokring_pause_hover.png"), _T("listctrl/working_pause_active.png") },
//				{ _T("delete"), _T("listctrl/working_delete_normal.png"), _T("listctrl/wokring_delete_hover.png"), _T("listctrl/working_delete_active.png") },
//				{ _T("search"), _T("listctrl/working_search_normal.png"), _T("listctrl/wokring_search_hover.png"), _T("listctrl/working_search_active.png") },
//			};
//
//			operation->ApplyAttributeList(
//				_T("name=\"operation\" visiable=\"true\" width=\"160\""));
//			CButtonUI * device_start = new CButtonUI;
//			TCHAR temp_attr[256];
//			_stprintf(temp_attr, 
//				_T("padding=\"0,1,0,0\" name=\"%s\" width=\"28\" height=\"28\" ")
//				_T("bkimage=\"%s\" ")
//				_T("hotimage=\"%s\" ")
//				_T("pushedimage=\"%s\""),
//				operation_icon[operation_id].icon_operation,
//				operation_icon[operation_id].icon_normal, operation_icon[operation_id].icon_hover, operation_icon[operation_id].icon_active);
//			device_start->ApplyAttributeList(temp_attr);
//			operation->Add(device_start);
//		}
//		new_h_lay->Add(operation);
//	}
//
//	new_node->Add(new_h_lay);
//	pRecordScheduleList->Add(new_node);
//}

void RecordScheduleSetOnline(CPaintManagerUI * m_pPaintManager, const char * uuid, BOOL bIsOnline)
{
	//assert(uuid != NULL);

	//CListUI * pRecordScheduleList = static_cast<CListUI*>(m_pPaintManager->FindControl(_T("basic_RecordSchedule_list")));
	//assert(pRecordScheduleList != NULL);

	//int count = pRecordScheduleList->GetCount();
	//for (int index = 0; index < count; index++)
	//{
	//	CListContainerElementUI * item = (CListContainerElementUI *)pRecordScheduleList->GetItemAt(index);
	//	CLabelUI * uuid_ctrl = (CLabelUI *)item->FindSubControl(_T("uuid"));

	//	if (uuid_ctrl == NULL) continue;

	//	TCHAR * w_uuid = mbs_to_wcs(uuid);
	//	LPCTSTR w_ctrl_uuid = uuid_ctrl->GetText().GetData();
	//	BOOL is_cur_uuid = _tcsicmp(w_ctrl_uuid, w_uuid) == 0;
	//	free(w_uuid);

	//	if (is_cur_uuid)
	//	{
	//		CButtonUI * online_btn = (CButtonUI *)item->FindSubControl(_T("RecordSchedule_online_btn"));

	//		TCHAR temp_attr[256];
	//		_stprintf(temp_attr, 
	//			_T("padding=\"23,8,0,0\" name=\"RecordSchedule_online_btn\" width=\"14\" height=\"14\" bkimage=\"%s\""),
	//			bIsOnline ? _T("listctrl/working_green.png") : _T("listctrl/working_red.png"));
	//		online_btn->ApplyAttributeList(temp_attr);
	//		break;
	//	}
	//}

}