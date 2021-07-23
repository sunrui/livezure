#pragma once

#include "MenuWnd.h"
#include "db_account_declare.h"

class WndBasicDevice : public CNotifyPump
{
public:
	WndBasicDevice(void);
	~WndBasicDevice(void);

public:
	void SetPaintMagager(CPaintManagerUI* pPaintMgr);
	void SetHwnd(HWND hWnd);

	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnMenu(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg );

public:
	void InitDeviceList();
	void DestroyDeviceList();

	void DeviceInsertToUI(db_account_device_t * device, BOOL bIsOnline);
	void DeviceUpdate(const char * uuid, db_account_device_t * device);
	void DeviceSetOnline(const char * uuid, BOOL bIsOnline);
	void DeviceDelete(const char * uuid);

protected:
	void OnDeviceCreate();
	void OnDeviceMofidy();
	void OnDeviceDetail();
	void OnDeviceDelete();

private:
	CPaintManagerUI* m_pPaintManager;
	CMenuWnd * m_pContextMenu;
	HWND m_hWnd;
};

