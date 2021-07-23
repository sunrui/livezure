#pragma once

#include "MenuWnd.h"
#include "db_account_declare.h"

class WndBasicAccount : public CNotifyPump
{
public:
	WndBasicAccount(void);
	~WndBasicAccount(void);

public:
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);
	void SetHwnd(HWND hWnd);

	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnMenu(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg);

public:
	void InitAccountList();
	void DestroyAccountList();

	void AccountInsertToUI(db_account_basic_t * account, BOOL bIsOnline);
	void AccountUpdate(const char * uuid, db_account_basic_t * account);
	void AccountSetOnline(const char * uuid, BOOL bIsOnline);
	void AccountDelete(const char * uuid);

public:
	void OnAccountCreate();
	void OnAccountModify();
	void OnAccountDelete();
	void OnAccountDetail();

private:
	CPaintManagerUI* m_pPaintManager;
	CMenuWnd * m_pContextMenu;
	HWND m_hWnd;
};

