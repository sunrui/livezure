#pragma once
#include "WndBasicAccount.h"
#include "guid.h"
#include "db_account.h"

class MainLogin : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	MainLogin()
	{
	}

	LPCTSTR GetWindowClassName() const { return _T("MainLogin UI"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{ 
		m_pm.RemovePreMessageFilter(this);
		delete this; 
	};

	void Init()
	{
		//CEditUI * edit_uuid = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_uuid")));
		//edit_uuid->SetEnabled(false);

		//CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
		//lable_error->SetVisible(false);

		//CLabelUI * lable_title = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_title")));
		//lable_title->SetText(_T("添加新设备"));
	}

	void DoLogin()
	{
		Close(IDYES);
		return;


		db_account_basic_t acc;
		TCHAR szErrorString[256];

		memset(&acc, 0, sizeof(acc));
		szErrorString[0] = _T('\0');

		// user name
		{
			CEditUI * edit_username = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_username")));
			LPCTSTR lpUserName;
			lpUserName = edit_username->GetText().GetData();
			if (lpUserName == NULL || _tcslen(lpUserName) == 0 || _tcslen(lpUserName) > 16)
			{
				_tcscpy(szErrorString, _T("用户名"));
				goto add_failed;
			}

			char * c_user;
			c_user = wcs_to_mbs(lpUserName);
			strncpy(acc.uuid, c_user, sizeof(acc.uuid) - 1);
			free(c_user);
		}

		// user pwd
		{
			CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
			CEditUI * edit_userpwd2 = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd2")));
			LPCTSTR lpUserPwd;
			lpUserPwd = edit_userpwd->GetText().GetData();
			if (lpUserPwd == NULL || _tcslen(lpUserPwd) == 0 || _tcslen(lpUserPwd) > 32)
			{
				_tcscpy(szErrorString, _T("密码"));
				goto add_failed;
			}
			LPCTSTR lpUserPwd2;
			lpUserPwd2 = edit_userpwd2->GetText().GetData();
			if (lpUserPwd2 == NULL || _tcslen(lpUserPwd2) == 0 || _tcslen(lpUserPwd2) > 32)
			{
				_tcscpy(szErrorString, _T("密码2"));
				goto add_failed;
			}

			if (_tcscmp(lpUserPwd, lpUserPwd2) != 0)
			{
				_tcscpy(szErrorString, _T("密码不一致"));
				goto add_failed;
			}

			char * c_pwd;
			c_pwd = wcs_to_mbs(lpUserPwd);
			strncpy(acc.pwd, c_pwd, sizeof(acc.pwd) - 1);
			free(c_pwd);
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
			strncpy(acc.nickname, c_nickname, sizeof(acc.nickname) - 1);
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
			strncpy(acc.comment, c_comment, sizeof(acc.comment) - 1);
			free(c_comment);
		}

		// check user whether exist
		{
			char ref_admin_uuid[17];
			bool r;

			r = db_account_check_user_exist(acc.uuid, NULL, ref_admin_uuid);
			if (r)
			{
				ASSERT(stricmp(ref_admin_uuid, "root") == 0);
				_tcscpy(szErrorString, _T("user exist"));
				goto add_failed;
			}
		}

		{
			char err[65];
			bool r;
			r = db_account_child_add("root", &acc, err);
			if (r)
			{
				db_account_basic_t ** childs;
				int count;
				r = db_account_list_child_get("root", acc.uuid, &childs, &count);
				ASSERT(r);
				if (!r)
				{
					TCHAR * werr;
					werr = mbs_to_wcs(err);
					_tcscpy(szErrorString, werr);
					free(werr);

					goto add_failed;
				}

				m_pWndBasicAccount->AccountInsertToUI(*childs, FALSE);

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
				DoLogin();
				return;
			}
		}
		else if (msg.sType == _T("itemselect"))
		{
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
		CControlUI* pRoot = builder.Create(_T("MainLogin.xml"), (UINT)0, NULL, &m_pm);
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
	WndBasicAccount * m_pWndBasicAccount;
	CPaintManagerUI m_pm;
};