#pragma once
#include "WndBasicDevice.h"
#include "guid.h"
#include "db_account.h"

class WndBasicDeviceDetail : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	WndBasicDeviceDetail(WndBasicDevice * pWndBasicDevice, db_account_device_t * device)
	{
		m_pWndBasicDevice = pWndBasicDevice;
		m_device = device;
	}

	LPCTSTR GetWindowClassName() const { return _T("WndBasicDeviceModify UI"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{
		m_pm.RemovePreMessageFilter(this);
		delete this; 
	};

	void Init()
	{
		// uuid
		{
			CEditUI * edit_uuid = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_uuid")));

			TCHAR * w_uuid;
			w_uuid = mbs_to_wcs(m_device->basic.uuid);
			edit_uuid->SetText(w_uuid);
			free(w_uuid);

			edit_uuid->SetReadOnly(true);
		}

		// plugin name
		{
			CComboUI * comb_pluginname = static_cast<CComboUI *>(m_pm.FindControl(_T("comb_pluginname")));
			char pluginnames[][16] = { "hb7000", "dahuasdk", "dalisdk", NULL };

			comb_pluginname->SelectItem(0);
			for (int plugin = 0; plugin < 3; plugin++)
			{
				if (strcmp(m_device->basic.plugin_name, pluginnames[plugin]) == 0)
				{
					comb_pluginname->SelectItem(plugin);
					break;
				}
			}

			comb_pluginname->SetEnabled(false);
		}

		// IP
		{
			CEditUI * edit_deviceip = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceip")));
			TCHAR * w_deviceip;

			w_deviceip = mbs_to_wcs(m_device->basic.ip);
			edit_deviceip->SetText(w_deviceip);
			free(w_deviceip);

			edit_deviceip->SetReadOnly(true);
		}

		// port
		{
			CEditUI * edit_deviceport = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceport")));
			TCHAR w_deviceport[32];

			_stprintf(w_deviceport, _T("%d"), m_device->basic.port);
			edit_deviceport->SetText(w_deviceport);

			edit_deviceport->SetReadOnly(true);
		}

		// user name
		{
			CEditUI * edit_username = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_username")));
			TCHAR * w_username;

			w_username = mbs_to_wcs(m_device->basic.user);
			edit_username->SetText(w_username);
			free(w_username);

			edit_username->SetReadOnly(true);
		}

		// pwd
		{
			CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
			TCHAR * w_userpwd;

			w_userpwd = mbs_to_wcs(m_device->basic.pwd);
			edit_userpwd->SetText(w_userpwd);
			free(w_userpwd);

			edit_userpwd->SetReadOnly(true);
		}

		// channel number
		{
			CEditUI * edit_channel = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_channel")));
			TCHAR w_channel[32];

			_stprintf(w_channel, _T("%d"), m_device->basic.channel);
			edit_channel->SetText(w_channel);

			edit_channel->SetReadOnly(true);
		}

		// nick name
		{
			CEditUI * edit_nickname = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_nickname")));
			TCHAR * w_nickname;

			w_nickname = mbs_to_wcs(m_device->nickname);
			edit_nickname->SetText(w_nickname);
			free(w_nickname);

			edit_nickname->SetReadOnly(true);
		}

		// comment
		{
			CEditUI * edit_comment = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_comment")));
			TCHAR * w_comment;

			w_comment = mbs_to_wcs(m_device->comment);
			edit_comment->SetText(w_comment);
			free(w_comment);

			edit_comment->SetReadOnly(true);
		}

		// working folder
		{
			CEditUI * edit_workingfolder = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_workingfolder")));
			TCHAR * w_workingfolder;

			w_workingfolder = mbs_to_wcs(m_device->basic.seg_folder);
			edit_workingfolder->SetText(w_workingfolder);
			free(w_workingfolder);

			edit_workingfolder->SetReadOnly(true);
		}

		CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
		lable_error->SetVisible(false);

		CLabelUI * lable_title = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_title")));
		lable_title->SetText(_T("设备详细信息"));
	}

	void Notify(TNotifyUI& msg)
	{
		if (msg.sType == _T("click"))
		{
			if (msg.pSender->GetName() == _T("closebtn") ||
				msg.pSender->GetName() == _T("cancel"))
			{
				Close(IDNO);
				return;
			}
			else if (msg.pSender->GetName() == _T("ok"))
			{
				Close(IDYES);
				return;
			}
		}
		else if (msg.sType == _T("itemselect"))
		{
			//if (msg.pSender->GetName() == _T("accountcombo")) {
			//	CEditUI* pAccountEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
			//	if (pAccountEdit) pAccountEdit->SetText(msg.pSender->GetText());
			//}
		}
		else if (msg.sType == _T("setfocus"))
		{
			CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
			lable_error->SetVisible(false);
		}
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		m_pm.AddPreMessageFilter(this);
		CDialogBuilder builder;
		//CDialogBuilderCallbackEx cb;
		CControlUI* pRoot = builder.Create(_T("WndBasicDeviceAdd.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		Init();
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (::IsIconic(*this)) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
				if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0)
					return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg) {
		case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
		}
		if (bHandled) return lRes;
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
	{
		if (uMsg == WM_KEYDOWN) {
			if (wParam == VK_RETURN) {
				//CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
				//if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
				//else {
				//	pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
				//	if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
				//	else Close();
				//}
				return true;
			}
			else if (wParam == VK_ESCAPE) {
				return true;
			}

		}
		return false;
	}

public:
	WndBasicDevice * m_pWndBasicDevice;
	db_account_device_t * m_device;
	CPaintManagerUI m_pm;
};