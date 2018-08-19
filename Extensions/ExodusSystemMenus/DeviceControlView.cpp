#include "DeviceControlView.h"
#include "resource.h"

//----------------------------------------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------------------------------------
DeviceControlView::DeviceControlView(IUIManager& uiManager, DeviceControlViewPresenter& presenter, ISystemGUIInterface& model)
:ViewBase(uiManager, presenter), _presenter(presenter), _model(model), _initializedDialog(false), _currentControlFocus(0)
{
	_deviceListIndex = -1;
	_systemStep = 0;
	_deviceStep = 1;
	SetDialogTemplateSettings(presenter.GetUnqualifiedViewTitle(), GetAssemblyHandle(), MAKEINTRESOURCE(IDD_DEVICECONTROL));
}

//----------------------------------------------------------------------------------------------------------------------
// Member window procedure
//----------------------------------------------------------------------------------------------------------------------
INT_PTR DeviceControlView::WndProcDialog(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	WndProcDialogImplementSaveFieldWhenLostFocus(hwnd, msg, wparam, lparam);
	switch (msg)
	{
	case WM_INITDIALOG:
		return msgWM_INITDIALOG(hwnd, wparam, lparam);
	case WM_TIMER:
		return msgWM_TIMER(hwnd, wparam, lparam);
	case WM_COMMAND:
		return msgWM_COMMAND(hwnd, wparam, lparam);
	}
	return FALSE;
}

//----------------------------------------------------------------------------------------------------------------------
// Event handlers
//----------------------------------------------------------------------------------------------------------------------
INT_PTR DeviceControlView::msgWM_INITDIALOG(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	SetTimer(hwnd, 1, 200, NULL);

	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------
INT_PTR DeviceControlView::msgWM_TIMER(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	_initializedDialog = true;

	// Update the textboxes
	if (_currentControlFocus != IDC_DEVICECONTROL_SYSTEM_EXECUTEAMOUNT)  UpdateDlgItemBin(hwnd, IDC_DEVICECONTROL_SYSTEM_EXECUTEAMOUNT, _systemStep);
	if (_currentControlFocus != IDC_DEVICECONTROL_DEVICE_STEPAMOUNT)     UpdateDlgItemBin(hwnd, IDC_DEVICECONTROL_DEVICE_STEPAMOUNT, _deviceStep);

	// Check if we need to refresh the device list
	std::list<IDevice*> newDeviceList = _model.GetLoadedDevices();
	bool refreshDeviceList = (newDeviceList.size() != _deviceList.size());
	if (!refreshDeviceList)
	{
		std::list<IDevice*>::const_iterator newDeviceListIterator = newDeviceList.begin();
		std::list<IDevice*>::const_iterator deviceListIterator = _deviceList.begin();
		while (!refreshDeviceList && (newDeviceListIterator != newDeviceList.end()) && (deviceListIterator != _deviceList.end()))
		{
			refreshDeviceList |= (*newDeviceListIterator != *deviceListIterator);
			++newDeviceListIterator;
			++deviceListIterator;
		}
	}

	// If we need to refresh the device list, copy the current list of devices into our
	// local copy.
	if (refreshDeviceList)
	{
		_deviceList = newDeviceList;
	}

	// Refresh the device list if required
	if (refreshDeviceList)
	{
		SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), WM_SETREDRAW, FALSE, 0);

		LRESULT top = SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETTOPINDEX, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_RESETCONTENT, 0, NULL);
		for (std::list<IDevice*>::const_iterator i = _deviceList.begin(); i != _deviceList.end(); ++i)
		{
			IDevice* device = *i;
			if (device->GetUpdateMethod() == IDevice::UpdateMethod::Step)
			{
				LRESULT newItemIndex = SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_ADDSTRING, 0, (LPARAM)device->GetDeviceInstanceName().Get().c_str());
				SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_SETITEMDATA, newItemIndex, (LPARAM)device);
			}
		}
		SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_SETCURSEL, (WPARAM)-1, 0);
		SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_SETTOPINDEX, top, 0);

		SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), WM_SETREDRAW, TRUE, 0);
		InvalidateRect(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), NULL, FALSE);

		_deviceListIndex = -1;
	}

	// Update the checkboxes
	int selectedDeviceIndex = (int)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETCURSEL, 0, NULL);
	LRESULT getItemDataResult = SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETITEMDATA, selectedDeviceIndex, NULL);
	if (getItemDataResult != LB_ERR)
	{
		IDevice* device = (IDevice*)getItemDataResult;
		if (device != 0)
		{
			CheckDlgButton(hwnd, IDC_DEVICECONTROL_DEVICE_ENABLED, device->GetDeviceContext()->DeviceEnabled()? BST_CHECKED: BST_UNCHECKED);
		}
	}

	return TRUE;
}

//----------------------------------------------------------------------------------------------------------------------
INT_PTR DeviceControlView::msgWM_COMMAND(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
	if (HIWORD(wparam) == BN_CLICKED)
	{
		switch (LOWORD(wparam))
		{
		case IDC_DEVICECONTROL_SYSTEM_RUN:
			_model.RunSystem();
			break;
		case IDC_DEVICECONTROL_SYSTEM_STOP:
			_model.StopSystem();
			break;
		case IDC_DEVICECONTROL_SYSTEM_EXECUTE:
			_model.ExecuteSystemStep(_systemStep);
			break;
		case IDC_DEVICECONTROL_DEVICE_INITIALIZE:{
			int selectedDeviceIndex = (int)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETCURSEL, 0, NULL);
			IDevice* device = (IDevice*)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETITEMDATA, selectedDeviceIndex, NULL);
			if (device != 0)
			{
				_model.InitializeDevice(device);
			}
			break;}
		case IDC_DEVICECONTROL_DEVICE_STEP:{
			int selectedDeviceIndex = (int)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETCURSEL, 0, NULL);
			IDevice* device = (IDevice*)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETITEMDATA, selectedDeviceIndex, NULL);
			if (device != 0)
			{
				//##TODO## Show a dialog box indicating the current step progress, and
				// giving the user an option to cancel.
				for (unsigned int i = 0; i < _deviceStep; ++i)
				{
					_model.ExecuteDeviceStep(device);
				}
			}
			break;}
		case IDC_DEVICECONTROL_DEVICE_ENABLED:{
			int selectedDeviceIndex = (int)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETCURSEL, 0, NULL);
			IDevice* device = (IDevice*)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETITEMDATA, selectedDeviceIndex, NULL);
			if (device != 0)
			{
				device->GetDeviceContext()->SetDeviceEnabled(IsDlgButtonChecked(hwnd, LOWORD(wparam)) == BST_CHECKED);
			}
			break;}
		}
	}
	else if ((HIWORD(wparam) == EN_SETFOCUS) && _initializedDialog)
	{
		_previousText = GetDlgItemString(hwnd, LOWORD(wparam));
		_currentControlFocus = LOWORD(wparam);
	}
	else if ((HIWORD(wparam) == EN_KILLFOCUS) && _initializedDialog)
	{
		std::wstring newText = GetDlgItemString(hwnd, LOWORD(wparam));
		if (newText != _previousText)
		{
			switch (LOWORD(wparam))
			{
			case IDC_DEVICECONTROL_DEVICE_STEPAMOUNT:
				_deviceStep = GetDlgItemBin(hwnd, LOWORD(wparam));
				break;
			case IDC_DEVICECONTROL_SYSTEM_EXECUTEAMOUNT:
				_systemStep = GetDlgItemBin(hwnd, LOWORD(wparam));
				break;
			}
		}
	}
	else if (HIWORD(wparam) == LBN_SELCHANGE)
	{
		switch (LOWORD(wparam))
		{
		case IDC_DEVICECONTROL_LIST:{
			int selectedItem = (int)SendMessage(GetDlgItem(hwnd, IDC_DEVICECONTROL_LIST), LB_GETCURSEL, 0, NULL);
			if (selectedItem != LB_ERR)
			{
				_deviceListIndex = selectedItem;
			}
			break;}
		}
	}

	return TRUE;
}
