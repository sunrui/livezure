#pragma once

#include "WndBasicDevice.h"
#include "WndBasicAccount.h"
#include "WndBasicRecordSchedule.h"

class MainFrame : public WindowImplBase
{
public:
	MainFrame(void);
	~MainFrame(void);

protected:
	virtual void InitWindow();
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	virtual LRESULT OnClose(UINT , WPARAM , LPARAM , BOOL& bHandled);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual void Notify(TNotifyUI& msg);

protected:
	void OnPrepare();
	void InitNavTree(void);
	BOOL InitDb(void);

private:
	WndBasicDevice m_wndBasicDevice;
	WndBasicAccount m_wndBasicAccount;
	WndBasicRecordSchedule m_wndBasicRecordSchedule;
};
