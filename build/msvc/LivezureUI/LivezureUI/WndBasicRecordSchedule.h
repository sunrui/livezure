#pragma once

class WndBasicRecordSchedule : public CNotifyPump
{
public:
	WndBasicRecordSchedule(void);
	~WndBasicRecordSchedule(void);

public:
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);
	void SetHwnd(HWND hWnd);

	DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg );

public:
	void InitRecordScheduleList();
	void DestroyRecordScheduleList();

private:
	CPaintManagerUI* m_pPaintManager;
	HWND m_hWnd;
};

