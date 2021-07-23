#pragma once
#include "WndBasicAccount.h"
#include "guid.h"
#include "db_account.h"

class WndBasicAccountDetail : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	WndBasicAccountDetail(WndBasicAccount * pWndBasicAccount, db_account_basic_t * account)
	{
		m_pWndBasicAccount = pWndBasicAccount;
		m_account = account;
	}

	LPCTSTR GetWindowClassName() const { return _T("WndBasicAccountDetail UI"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{ 
		m_pm.RemovePreMessageFilter(this);
		delete this; 
	};

	void Init()
	{
		CLabelUI * lable_title = static_cast<CLabelUI *>(m_pm.FindControl(_T("lable_title")));
		lable_title->SetText(_T("用户详细信息"));

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

			edit_userpwd->SetEnabled(false);
			edit_userpwd2->SetEnabled(false);
		}

		// nickname
		{
			CEditUI * edit_nickname = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_nickname")));
			TCHAR * w_nickname;

			w_nickname = mbs_to_wcs(m_account->nickname);
			edit_nickname->SetText(w_nickname);
			free(w_nickname);

			edit_nickname->SetEnabled(false);
		}

		// comment
		{
			CEditUI * edit_comment = static_cast<CEditUI *>(m_pm.FindControl(_T("edit_comment")));
			TCHAR * w_comment;

			w_comment = mbs_to_wcs(m_account->comment);
			edit_comment->SetText(w_comment);
			free(w_comment);

			edit_comment->SetEnabled(false);
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
		}
		else if (msg.sType == _T("killfocus"))
		{
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