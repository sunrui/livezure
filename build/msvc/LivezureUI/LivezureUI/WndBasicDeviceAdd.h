#pragma once
#include "WndBasicDevice.h"
#include "guid.h"
#include "db_account.h"
//
//class WndBasicDeviceAdd : public WindowImplBase
//{
//public:
//	WndBasicDeviceAdd(void);
//	~WndBasicDeviceAdd(void);
//
//protected:
//	virtual void InitWindow();
//	virtual CDuiString GetSkinFolder();
//	virtual CDuiString GetSkinFile();
//	virtual LPCTSTR GetWindowClassName(void) const;
//	virtual LRESULT OnClose(UINT , WPARAM , LPARAM , BOOL& bHandled);
//	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
//	virtual void Notify(TNotifyUI& msg);
//	virtual void OnFinalMessage(HWND hWnd);
//
//protected:
//
//private:
//	CPaintManagerUI m_pm;
//};

class WndBasicDeviceAdd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	WndBasicDeviceAdd(WndBasicDevice * pWndBasicDevice)
	{
		m_pWndBasicDevice = pWndBasicDevice;
	}

	LPCTSTR GetWindowClassName() const { return _T("WndBasicDeviceAdd UI"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{ 
		m_pm.RemovePreMessageFilter(this);
		delete this; 
	};

	void Init()
	{
		CEditUI * edit_uuid = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_uuid")));
		edit_uuid->SetEnabled(false);

		CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
		lable_error->SetVisible(false);

		CLabelUI * lable_title = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_title")));
		lable_title->SetText(_T("添加新设备"));
	}

	void DoAddDevice()
	{
		db_account_device_t ac_dec;
		TCHAR szErrorString[256];

		memset(&ac_dec, 0, sizeof(ac_dec));
		szErrorString[0] = _T('\0');

		// uuid
		{
			CEditUI * edit_uuid = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_uuid")));

			// uuid 随机生成
			char uuid[39];
			guid_generate(uuid);

			strncpy(ac_dec.basic.uuid, uuid,sizeof(ac_dec.basic.uuid) - 1);
		}

		// plugin name
		{
			CComboUI * comb_pluginname = static_cast<CComboUI *>(m_pm.FindControl(_T("comb_pluginname")));
			char pluginnames[][16] = { "hb7000", "dahuasdk", "dalisdk", NULL };

			int nPlugin = comb_pluginname->GetCurSel();
			if (nPlugin > 3) nPlugin = 3;
			const char * plugin = pluginnames[nPlugin];

			if (plugin == NULL)
			{
				_tcscpy(szErrorString, _T("plugin"));
				goto add_failed;
			}
			
			strncpy(ac_dec.basic.plugin_name, plugin, sizeof(ac_dec.basic.plugin_name) - 1);
		}

		// ip
		{
			CEditUI * edit_deviceip = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceip")));
			LPCTSTR lpIp;
			lpIp = edit_deviceip->GetText().GetData();
			if (lpIp == NULL || _tcslen(lpIp) == 0 || _tcslen(lpIp) > 16)
			{
				_tcscpy(szErrorString, _T("设备 IP"));
				goto add_failed;
			}

			char * c_ip;
			c_ip = wcs_to_mbs(lpIp);
			strncpy(ac_dec.basic.ip, c_ip, sizeof(ac_dec.basic.ip) - 1);
			free(c_ip);
		}

		// port
		{
			CEditUI * edit_deviceport = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_deviceport")));
			LPCTSTR lpPort;
			lpPort = edit_deviceport->GetText().GetData();
			if (lpPort == NULL || _tcslen(lpPort) == 0 || _tcslen(lpPort) > 32)
			{
				_tcscpy(szErrorString, _T("端口"));
				goto add_failed;
			}

			char * c_port;
			c_port = wcs_to_mbs(lpPort);
			int nport = atoi(c_port);
			free(c_port);

			ac_dec.basic.port = nport;
		}

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
			strncpy(ac_dec.basic.user, c_user, sizeof(ac_dec.basic.user) - 1);
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
			strncpy(ac_dec.basic.pwd, c_pwd, sizeof(ac_dec.basic.pwd) - 1);
			free(c_pwd);
		}

		// channel number
		{
			CEditUI * edit_channel = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_channel")));
			LPCTSTR lpChannel;
			lpChannel = edit_channel->GetText().GetData();
			if (lpChannel == NULL || _tcslen(lpChannel) == 0 || _tcslen(lpChannel) > 32)
			{
				_tcscpy(szErrorString, _T("通道号"));
				goto add_failed;
			}

			char * c_channel;
			c_channel = wcs_to_mbs(lpChannel);
			int nchannel = atoi(c_channel);
			free(c_channel);

			ac_dec.basic.channel = nchannel;
		}

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
			strncpy(ac_dec.nickname, c_nickname, sizeof(ac_dec.nickname) - 1);
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
			strncpy(ac_dec.comment, c_comment, sizeof(ac_dec.comment) - 1);
			free(c_comment);
		}

		// working folder
		{
			CEditUI * edit_workingfolder = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_workingfolder")));
			LPCTSTR lpWorkingFolder;
			lpWorkingFolder = edit_workingfolder->GetText().GetData();
			if (lpWorkingFolder == NULL || _tcslen(lpWorkingFolder) == 0 || _tcslen(lpWorkingFolder) > 64)
			{
				_tcscpy(szErrorString, _T("工作目录"));
				goto add_failed;
			}

			char * c_workingfolder;
			c_workingfolder = wcs_to_mbs(lpWorkingFolder);
			strncpy(ac_dec.basic.seg_folder, c_workingfolder, sizeof(ac_dec.basic.seg_folder) - 1);
			free(c_workingfolder);
		}

		ac_dec.basic.seg_in_count = 3; /* how many segmenter files in one seg_file */
		ac_dec.basic.seg_per_sec = 10; /* segmenter per-second */
		ac_dec.is_top_level = 1;
		ac_dec.ref_parent_uuid[0] = 0;

		{
			char err[65];
			bool r;
			r = db_account_device_add("root", &ac_dec, err);
			if (r)
			{
				db_account_device_t ** device;
				int count;
				r = db_account_device_get("root", ac_dec.basic.uuid, &device, &count);
				ASSERT(r);
				if (!r)
				{
					TCHAR * werr;
					werr = mbs_to_wcs(err);
					_tcscpy(szErrorString, werr);
					free(werr);

					goto add_failed;
				}

				m_pWndBasicDevice->DeviceInsertToUI(*device, FALSE);

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
				DoAddDevice();
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
	CPaintManagerUI m_pm;
};