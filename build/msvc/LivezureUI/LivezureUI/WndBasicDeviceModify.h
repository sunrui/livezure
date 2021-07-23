#pragma once
#include "WndBasicDevice.h"
#include "guid.h"
#include "db_account.h"

class WndBasicDeviceModify : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	WndBasicDeviceModify(WndBasicDevice * pWndBasicDevice, db_account_device_t * device)
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
			edit_uuid->SetEnabled(false);

			TCHAR * w_uuid;
			w_uuid = mbs_to_wcs(m_device->basic.uuid);
			edit_uuid->SetText(w_uuid);
			free(w_uuid);
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

			edit_deviceip->SetEnabled(false);
		}

		// port
		{
			CEditUI * edit_deviceport = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceport")));
			TCHAR w_deviceport[32];

			_stprintf(w_deviceport, _T("%d"), m_device->basic.port);
			edit_deviceport->SetText(w_deviceport);

			edit_deviceport->SetEnabled(false);
		}

		// user name
		{
			CEditUI * edit_username = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_username")));
			TCHAR * w_username;

			w_username = mbs_to_wcs(m_device->basic.user);
			edit_username->SetText(w_username);
			free(w_username);
		}

		// pwd
		{
			CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
			TCHAR * w_userpwd;

			w_userpwd = mbs_to_wcs(m_device->basic.pwd);
			edit_userpwd->SetText(w_userpwd);
			free(w_userpwd);
		}

		// channel number
		{
			CEditUI * edit_channel = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_channel")));
			TCHAR w_channel[32];

			_stprintf(w_channel, _T("%d"), m_device->basic.channel);
			edit_channel->SetText(w_channel);

			edit_channel->SetEnabled(false);
		}

		// nick name
		{
			CEditUI * edit_nickname = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_nickname")));
			TCHAR * w_nickname;

			w_nickname = mbs_to_wcs(m_device->nickname);
			edit_nickname->SetText(w_nickname);
			free(w_nickname);
		}

		// comment
		{
			CEditUI * edit_comment = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_comment")));
			TCHAR * w_comment;

			w_comment = mbs_to_wcs(m_device->comment);
			edit_comment->SetText(w_comment);
			free(w_comment);
		}

		// working folder
		{
			CEditUI * edit_workingfolder = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_workingfolder")));
			TCHAR * w_workingfolder;

			w_workingfolder = mbs_to_wcs(m_device->basic.seg_folder);
			edit_workingfolder->SetText(w_workingfolder);
			free(w_workingfolder);
		}

		CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
		lable_error->SetVisible(false);

		CLabelUI * lable_title = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_title")));
		lable_title->SetText(_T("修改设备"));
	}

	void DoModifyDevice()
	{
		TCHAR szErrorString[256];
		BOOL bChanged = FALSE;

		szErrorString[0] = _T('\0');

		//// uuid
		//{
		//	CEditUI * edit_uuid = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_uuid")));

		//	// uuid 随机生成
		//	char uuid[39];
		//	guid_generate(uuid);

		//	strncpy(ac_dec.basic.uuid, uuid,sizeof(ac_dec.basic.uuid) - 1);
		//}

		//// plugin name
		//{
		//	CComboUI * comb_pluginname = static_cast<CComboUI *>(m_pm.FindControl(_T("comb_pluginname")));
		//	char pluginnames[][16] = { "hb7000", "dahuasdk", "dalisdk", NULL };

		//	int nPlugin = comb_pluginname->GetCurSel();
		//	if (nPlugin > 3) nPlugin = 3;
		//	const char * plugin = pluginnames[nPlugin];

		//	if (plugin == NULL)
		//	{
		//		_tcscpy(szErrorString, _T("plugin"));
		//		goto add_failed;
		//	}

		//	strncpy(ac_dec.basic.plugin_name, plugin, sizeof(ac_dec.basic.plugin_name) - 1);
		//}

		//// ip
		//{
		//	CEditUI * edit_deviceip = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceip")));
		//	LPCTSTR lpIp;
		//	lpIp = edit_deviceip->GetText().GetData();
		//	if (lpIp == NULL || _tcslen(lpIp) == 0 || _tcslen(lpIp) > 16)
		//	{
		//		_tcscpy(szErrorString, _T("设备 IP"));
		//		goto add_failed;
		//	}

		//	char * c_ip;
		//	c_ip = wcs_to_mbs(lpIp);
		//	strncpy(ac_dec.basic.ip, c_ip, sizeof(ac_dec.basic.ip) - 1);
		//	free(c_ip);
		//}

		//// port
		//{
		//	CEditUI * edit_deviceport = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceport")));
		//	LPCTSTR lpPort;
		//	lpPort = edit_deviceport->GetText().GetData();
		//	if (lpPort == NULL || _tcslen(lpPort) == 0 || _tcslen(lpPort) > 32)
		//	{
		//		_tcscpy(szErrorString, _T("端口"));
		//		goto add_failed;
		//	}

		//	char * c_port;
		//	c_port = wcs_to_mbs(lpPort);
		//	int nport = atoi(c_port);
		//	free(c_port);

		//	ac_dec.basic.port = nport;
		//}

		// user name
		{
			CEditUI * edit_username = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_username")));
			LPCTSTR lpUserName;
			lpUserName = edit_username->GetText().GetData();
			if (lpUserName == NULL || _tcslen(lpUserName) == 0 || _tcslen(lpUserName) > 32)
			{
				_tcscpy(szErrorString, _T("用户名"));
				goto add_failed;
			}

			char * c_user;
			c_user = wcs_to_mbs(lpUserName);

			if (strcmp(m_device->basic.user, c_user) != 0)
			{
				strncpy(m_device->basic.user, c_user, sizeof(m_device->basic.user) - 1);
				bChanged = TRUE;
			}
			free(c_user);
		}

		// user pwd
		{
			CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
			LPCTSTR lpUserPwd;
			lpUserPwd = edit_userpwd->GetText().GetData();
			if (lpUserPwd == NULL || _tcslen(lpUserPwd) == 0 || _tcslen(lpUserPwd) > 32)
			{
				_tcscpy(szErrorString, _T("密码"));
				goto add_failed;
			}

			char * c_pwd;
			c_pwd = wcs_to_mbs(lpUserPwd);
			if (strcmp(m_device->basic.pwd, c_pwd) != 0)
			{
				strncpy(m_device->basic.pwd, c_pwd, sizeof(m_device->basic.pwd) - 1);
				bChanged = TRUE;
			}
			free(c_pwd);
		}

		//// channel number
		//{
		//	CEditUI * edit_channel = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_channel")));
		//	LPCTSTR lpChannel;
		//	lpChannel = edit_channel->GetText().GetData();
		//	if (lpChannel == NULL || _tcslen(lpChannel) == 0 || _tcslen(lpChannel) > 32)
		//	{
		//		_tcscpy(szErrorString, _T("通道号"));
		//		goto add_failed;
		//	}

		//	char * c_channel;
		//	c_channel = wcs_to_mbs(lpChannel);
		//	int nchannel = atoi(c_channel);
		//	free(c_channel);

		//	ac_dec.basic.channel = nchannel;
		//}

		// nickname
		{
			CEditUI * edit_nickname = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_nickname")));
			LPCTSTR lpNickName;
			lpNickName = edit_nickname->GetText().GetData();
			if (lpNickName == NULL || _tcslen(lpNickName) == 0 || _tcslen(lpNickName) > 64)
			{
				_tcscpy(szErrorString, _T("昵称"));
				goto add_failed;
			}

			char * c_nickname;
			c_nickname = wcs_to_mbs(lpNickName);
			if (strcmp(m_device->nickname, c_nickname) != 0)
			{
				strncpy(m_device->nickname, c_nickname, sizeof(m_device->nickname) - 1);
				bChanged = TRUE;
			}
			free(c_nickname);
		}

		// comment
		{
			CEditUI * edit_comment = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_comment")));
			LPCTSTR lpComment;
			lpComment = edit_comment->GetText().GetData();
			if (lpComment == NULL || _tcslen(lpComment) == 0 || _tcslen(lpComment) > 64)
			{
				_tcscpy(szErrorString, _T("备注"));
				goto add_failed;
			}

			char * c_comment;
			c_comment = wcs_to_mbs(lpComment);
			if (strcmp(c_comment, m_device->comment) != 0)
			{
				strncpy(m_device->comment, c_comment, sizeof(m_device->comment) - 1);
				bChanged = TRUE;
			}
			free(c_comment);
		}

		// working folder
		{
			CEditUI * edit_workingfolder = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_workingfolder")));
			LPCTSTR lpWorkingFolder;
			lpWorkingFolder = edit_workingfolder->GetText().GetData();
			if (lpWorkingFolder == NULL || _tcslen(lpWorkingFolder) == 0 || _tcslen(lpWorkingFolder) > 64)
			{
				_tcscpy(szErrorString, _T("备注"));
				goto add_failed;
			}

			char * c_workingfolder;
			c_workingfolder = wcs_to_mbs(lpWorkingFolder);
			if (strcmp(c_workingfolder, m_device->basic.seg_folder) != 0)
			{
				strncpy(m_device->basic.seg_folder, c_workingfolder, sizeof(m_device->basic.seg_folder) - 1);
				bChanged = TRUE;
			}
			free(c_workingfolder);
		}

		if (!bChanged)
		{
			Close(IDCANCEL);
			return;
		}

		m_device->basic.seg_in_count = 3; /* how many segmenter files in one seg_file */
		m_device->basic.seg_per_sec = 10; /* segmenter per-second */
		m_device->is_top_level = 1;
		m_device->ref_parent_uuid[0] = 0;

		{
			char err[65];
			bool r;

			r = db_account_device_del("root", m_device->basic.uuid);
			ASSERT(r);
			r = db_account_device_add("root", m_device, err);
			if (r)
			{
				m_pWndBasicDevice->DeviceUpdate(m_device->basic.uuid, m_device);
				Close(IDYES);
				return;
			}
			else
			{
				TCHAR * werr;
				werr = mbs_to_wcs(err);
				_tcscpy(szErrorString, werr);
				free(werr);
			}
			return;
		}

add_failed:
		{
			CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
			lable_error->SetText(szErrorString);
			lable_error->SetVisible(true);
		}
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
				DoModifyDevice();
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