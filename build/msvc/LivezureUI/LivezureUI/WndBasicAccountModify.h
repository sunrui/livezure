#pragma once
#include "WndBasicAccount.h"
#include "guid.h"
#include "db_account.h"

class WndBasicAccountModify : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	WndBasicAccountModify(WndBasicAccount * pWndBasicAccount, db_account_basic_t * account)
	{
		m_pWndBasicAccount = pWndBasicAccount;
		m_account = account;
	}

	LPCTSTR GetWindowClassName() const { return _T("WndBasicAccountAdd UI"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{ 
		m_pm.RemovePreMessageFilter(this);
		delete this; 
	};

	void Init()
	{
		CLabelUI * lable_title = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_title")));
		lable_title->SetText(_T("修改用户"));

		CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
		lable_error->SetVisible(false);

		// user name
		{
			CEditUI * edit_username = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_username")));
			TCHAR * w_username;

			w_username = mbs_to_wcs(m_account->uuid);
			edit_username->SetText(w_username);
			free(w_username);

			edit_username->SetEnabled(false);
		}

		// user pwd
		{
			CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
			CEditUI * edit_userpwd2 = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd2")));

			edit_userpwd->SetText(_T("未修改"));
			edit_userpwd2->SetText(_T("未修改"));
			edit_userpwd->SetTextColor(0xFFA7A6AA);
			edit_userpwd2->SetTextColor(0xFFA7A6AA);
			m_bIsPwdUpdated = FALSE;
			m_bIsPwdUpdated2 = FALSE;
		}

		// nickname
		{
			CEditUI * edit_nickname = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_nickname")));
			TCHAR * w_nickname;

			w_nickname = mbs_to_wcs(m_account->nickname);
			edit_nickname->SetText(w_nickname);
			free(w_nickname);
		}

		// comment
		{
			CEditUI * edit_comment = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_comment")));
			TCHAR * w_comment;

			w_comment = mbs_to_wcs(m_account->comment);
			edit_comment->SetText(w_comment);
			free(w_comment);
		}
	}

	void DoModifyAccount()
	{
		TCHAR szErrorString[256];
		BOOL bChanged = FALSE;

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

			if (strcmp(c_user, m_account->uuid) != 0)
			{
				strncpy(m_account->uuid, c_user, sizeof(m_account->uuid) - 1);
				bChanged = TRUE;
			}
			free(c_user);
		}

		// user pwd
		{
			CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
			CEditUI * edit_userpwd2 = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd2")));
			LPCTSTR lpUserPwd;

			if (m_bIsPwdUpdated || m_bIsPwdUpdated2)
			{
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

				if (strcmp(c_pwd, m_account->pwd) != 0)
				{
					strncpy(m_account->pwd, c_pwd, sizeof(m_account->pwd) - 1);
					bChanged = TRUE;
				}
				free(c_pwd);
			}
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

			if (strcmp(c_nickname, m_account->nickname) != 0)
			{
				strncpy(m_account->nickname, c_nickname, sizeof(m_account->nickname) - 1);
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
			if (strcmp(c_comment, m_account->comment) != 0)
			{
				strncpy(m_account->comment, c_comment, sizeof(m_account->comment) - 1);
				bChanged = TRUE;
			}
			free(c_comment);
		}

		if (!bChanged)
		{
			Close(IDCANCEL);
			return;
		}

		// check user whether exist
		{
			char ref_admin_uuid[17];
			bool r;

			r = db_account_check_user_exist(m_account->uuid, NULL, ref_admin_uuid);
			if (!r)
			{
				ASSERT(0);
				_tcscpy(szErrorString, _T("internal error"));
				goto add_failed;
			}
		}

		{
			char err[65];
			bool r;
			r = db_account_update(m_account->uuid, m_account, err);
			if (r)
			{
				db_account_basic_t ** childs;
				int count;
				r = db_account_list_child_get("root", m_account->uuid, &childs, &count);
				ASSERT(r);
				if (!r)
				{
					TCHAR * werr;
					werr = mbs_to_wcs(err);
					_tcscpy(szErrorString, werr);
					free(werr);

					goto add_failed;
				}

				m_pWndBasicAccount->AccountUpdate(m_account->uuid, *childs);
				xfree(childs);
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
				DoModifyAccount();
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

			if (msg.pSender->GetName() == _T("edit_userpwd") ||
				msg.pSender->GetName() == _T("edit_userpwd2"))
			{
				CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
				CEditUI * edit_userpwd2 = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd2")));

				if (!m_bIsPwdUpdated)
				{
					edit_userpwd->SetText(_T(""));
					m_bIsPwdUpdated = TRUE;
					edit_userpwd->SetTextColor(0xFF000000);
				}

				if (!m_bIsPwdUpdated2)
				{
					edit_userpwd2->SetText(_T(""));
					m_bIsPwdUpdated2 = TRUE;
					edit_userpwd2->SetTextColor(0xFF000000);
				}
			}
		}
		else if (msg.sType == _T("killfocus"))
		{
			CLabelUI * lable_error = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_error")));
			lable_error->SetVisible(false);

			if (msg.pSender->GetName() == _T("edit_userpwd") ||
				msg.pSender->GetName() == _T("edit_userpwd2"))
			{
				CEditUI * edit_userpwd = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd")));
				CEditUI * edit_userpwd2 = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_userpwd2")));

				if (edit_userpwd->GetText().IsEmpty() &&
					edit_userpwd2->GetText().IsEmpty())
				{
					edit_userpwd->SetText(_T("未修改"));
					edit_userpwd->SetTextColor(0xFFA7A6AA);
					edit_userpwd2->SetText(_T("未修改"));
					edit_userpwd2->SetTextColor(0xFFA7A6AA);
					m_bIsPwdUpdated = FALSE;
					m_bIsPwdUpdated2 = FALSE;
				}
			}
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
		CControlUI* pRoot = builder.Create(_T("WndBasicAccountAdd.xml"), (UINT)0, NULL, &m_pm);
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
	db_account_basic_t * m_account;
	BOOL m_bIsPwdUpdated;
	BOOL m_bIsPwdUpdated2;
};