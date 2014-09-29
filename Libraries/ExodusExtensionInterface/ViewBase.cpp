#include "ViewBase.h"

//----------------------------------------------------------------------------------------
//Constructors
//----------------------------------------------------------------------------------------
ViewBase::ViewBase(IUIManager& auiManager, IViewPresenter& aviewPresenter)
:hwndInternal(NULL), parentWindow(0), uiManager(auiManager), viewPresenter(aviewPresenter), assemblyHandle(0), dialogSettingsCaptured(false), windowSettingsCaptured(false), initiallyDocked(false), initiallyCollapsed(false), windowShownForFirstTime(false), viewType(ViewType::Dockable), initialDialogPos(DialogPos::Default)
{}

//----------------------------------------------------------------------------------------
//Interface version functions
//----------------------------------------------------------------------------------------
unsigned int ViewBase::GetIViewVersion() const
{
	return ThisIViewVersion();
}

//----------------------------------------------------------------------------------------
//Window settings
//----------------------------------------------------------------------------------------
void ViewBase::SetWindowSettings(const std::wstring& ainitialWindowTitle, DWORD awindowStyle, DWORD aextendedWindowStyle, unsigned int awidth, unsigned int aheight)
{
	windowSettingsCaptured = true;
	initialWindowTitle = ainitialWindowTitle;
	windowStyle = awindowStyle;
	extendedWindowStyle = aextendedWindowStyle;
	originalWindowWidth = awidth;
	originalWindowHeight = aheight;
}

//----------------------------------------------------------------------------------------
void ViewBase::SetDialogTemplateSettings(const std::wstring& ainitialWindowTitle, void* aassemblyHandle, LPCWSTR atemplateName)
{
	dialogSettingsCaptured = true;
	initialWindowTitle = ainitialWindowTitle;
	assemblyHandle = aassemblyHandle;
	templateName = atemplateName;
}

//----------------------------------------------------------------------------------------
void ViewBase::SetDocumentViewType()
{
	viewType = ViewType::Document;
}

//----------------------------------------------------------------------------------------
void ViewBase::SetDockableViewType(bool ainitiallyDocked, DockPos ainitialDockPos, bool ainitiallyCollapsed, const std::wstring& aviewDockingGroup)
{
	viewType = ViewType::Dockable;
	viewDockingGroup = aviewDockingGroup;
	initiallyDocked = ainitiallyDocked;
	initialDockPos = ainitialDockPos;
	initiallyCollapsed = ainitiallyCollapsed;
}

//----------------------------------------------------------------------------------------
void ViewBase::SetDialogViewType(DialogMode adialogMode, bool adialogResizable, DialogPos ainitialDialogPos)
{
	viewType = ViewType::Dialog;
	dialogMode = adialogMode;
	dialogResizable = adialogResizable;
	initialDialogPos = ainitialDialogPos;
}

//----------------------------------------------------------------------------------------
//View management functions
//----------------------------------------------------------------------------------------
bool ViewBase::OpenView(IHierarchicalStorageNode* viewState)
{
	//Create the native window for this view. Note that the window will initially appear
	//hidden.
	hwndInternal = NULL;
	if(dialogSettingsCaptured)
	{
		hwndInternal = uiManager.CreateDialogWindow(*this, viewPresenter, (HINSTANCE)assemblyHandle, WndProcDialogInternal, templateName);
	}
	else if(windowSettingsCaptured)
	{
		hwndInternal = uiManager.CreateNativeWindow(*this, viewPresenter, (HINSTANCE)viewPresenter.GetAssemblyHandle(), WndProcWindowInternal, windowStyle, extendedWindowStyle);
	}
	if(hwndInternal == NULL)
	{
		return false;
	}

	//If we're opening a native window rather than a dialog, resize the window to the
	//requested window size.
	if(!dialogSettingsCaptured && windowSettingsCaptured)
	{
		uiManager.ResizeWindowToTargetClientSize(*this, viewPresenter, hwndInternal, originalWindowWidth, originalWindowHeight);
	}

	//If a view state has been defined to restore, load it now, otherwise show the window.
	if(viewState != 0)
	{
		//Attempt to load the state for this view. This will also show the view with the
		//saved state for the window.
		if(!LoadViewState(*viewState))
		{
			uiManager.CloseWindow(*this, viewPresenter, hwndInternal);
			return false;
		}
	}
	else
	{
		//Request the UI manager to show this view for the first time
		if(!uiManager.ShowWindowFirstTime(*this, viewPresenter, hwndInternal, initialWindowTitle))
		{
			uiManager.CloseWindow(*this, viewPresenter, hwndInternal);
			return false;
		}
		windowShownForFirstTime = true;
	}

	//Return the result to the caller
	return true;
}

//----------------------------------------------------------------------------------------
void ViewBase::CloseView()
{
	if(hwndInternal != NULL)
	{
		uiManager.CloseWindow(*this, viewPresenter, hwndInternal);
	}
}

//----------------------------------------------------------------------------------------
void ViewBase::ShowView()
{
	if(hwndInternal != NULL)
	{
		uiManager.ShowWindow(*this, viewPresenter, hwndInternal);
	}
}

//----------------------------------------------------------------------------------------
void ViewBase::HideView()
{
	if(hwndInternal != NULL)
	{
		uiManager.HideWindow(*this, viewPresenter, hwndInternal);
	}
}

//----------------------------------------------------------------------------------------
void ViewBase::ActivateView()
{
	if(hwndInternal != NULL)
	{
		uiManager.ActivateWindow(*this, viewPresenter, hwndInternal);
	}
}

//----------------------------------------------------------------------------------------
//State functions
//----------------------------------------------------------------------------------------
bool ViewBase::LoadViewState(IHierarchicalStorageNode& viewState)
{
	//Ensure the window has been created
	if(hwndInternal == NULL)
	{
		return false;
	}

	//Load the window state for this view if present
	IHierarchicalStorageNode* windowState = viewState.GetChild(L"WindowState");
	if(!windowShownForFirstTime)
	{
		if(!uiManager.ShowWindowFirstTime(*this, viewPresenter, hwndInternal, initialWindowTitle, windowState))
		{
			return false;
		}
		windowShownForFirstTime = true;
	}
	else
	{
		if(windowState != 0)
		{
			if(!uiManager.LoadWindowState(*this, viewPresenter, hwndInternal, *windowState))
			{
				return false;
			}
		}
	}

	//Return the result to the caller
	return true;
}

//----------------------------------------------------------------------------------------
bool ViewBase::SaveViewState(IHierarchicalStorageNode& viewState) const
{
	//Ensure the window has been created
	if(hwndInternal == NULL)
	{
		return false;
	}

	//Save the window state for this view
	return uiManager.SaveWindowState(*this, viewPresenter, hwndInternal, viewState.CreateChild(L"WindowState"));
}

//----------------------------------------------------------------------------------------
//New window state
//----------------------------------------------------------------------------------------
std::wstring ViewBase::GetViewDockingGroup() const
{
	return viewDockingGroup;
}

//----------------------------------------------------------------------------------------
void ViewBase::GetViewDockingGroupInternal(const InteropSupport::ISTLObjectTarget<std::wstring>& marshaller) const
{
	marshaller.MarshalFrom(GetViewDockingGroup());
}

//----------------------------------------------------------------------------------------
bool ViewBase::IsViewInitiallyDocked() const
{
	return initiallyDocked;
}

//----------------------------------------------------------------------------------------
bool ViewBase::IsViewInitiallyCollapsed() const
{
	return initiallyCollapsed;
}

//----------------------------------------------------------------------------------------
ViewBase::DockPos ViewBase::GetViewInitialDockPosition() const
{
	return initialDockPos;
}

//----------------------------------------------------------------------------------------
ViewBase::ViewType ViewBase::GetViewType() const
{
	return viewType;
}

//----------------------------------------------------------------------------------------
ViewBase::DialogMode ViewBase::GetViewDialogMode() const
{
	return dialogMode;
}

//----------------------------------------------------------------------------------------
ViewBase::DialogPos ViewBase::GetViewInitialDialogPosition() const
{
	return initialDialogPos;
}

//----------------------------------------------------------------------------------------
bool ViewBase::CanResizeDialog() const
{
	return dialogResizable;
}

//----------------------------------------------------------------------------------------
//Helper functions
//----------------------------------------------------------------------------------------
HINSTANCE ViewBase::GetAssemblyHandle() const
{
	return (HINSTANCE)viewPresenter.GetAssemblyHandle();
}

//----------------------------------------------------------------------------------------
//##TODO## Remove this
std::wstring ViewBase::GetLockingSupportedMessage() const
{
	return L"Selected controls on this window support register locking. "
	       L"Register locking allows the current value of a register to "
	       L"be frozen, preventing changes from within the system. "
	       L"Attempts by any device to modify locked registers will be "
	       L"ignored. The current value of locked registers can still be "
	       L"modified through the debugger. To toggle locking for a "
	       L"register, hold the CTRL key while clicking on a control. "
	       L"Controls that support locking will change colour to indicate "
	       L"when they are locked.";
}

//----------------------------------------------------------------------------------------
//Member window procedure
//----------------------------------------------------------------------------------------
INT_PTR ViewBase::WndProcDialog(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return 0;
}

//----------------------------------------------------------------------------------------
LRESULT ViewBase::WndProcWindow(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return 0;
}

//----------------------------------------------------------------------------------------
//Window procedure helper functions
//----------------------------------------------------------------------------------------
void ViewBase::WndProcDialogImplementSaveFieldWhenLostFocus(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
	//Make sure no textbox is selected on startup, and remove focus from textboxes when
	//the user clicks an unused area of the window.
	case WM_LBUTTONDOWN:
	case WM_SHOWWINDOW:
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), NULL);
		SetFocus(NULL);
		break;
	}
}

//----------------------------------------------------------------------------------------
//Static window procedure
//----------------------------------------------------------------------------------------
INT_PTR CALLBACK ViewBase::WndProcDialogInternal(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//Obtain the object pointer
	ViewBase* state = (ViewBase*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	//Process the message
	switch(msg)
	{
	case WM_INITDIALOG:
		//Set the object pointer
		state = (ViewBase*)lparam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(state));

		//Pass this message on to the member window procedure function
		if(state != 0)
		{
			return state->WndProcDialog(hwnd, msg, wparam, lparam);
		}
		break;
	case WM_CLOSE:
		if(state != 0)
		{
			//Pass this message on to the member window procedure function
			INT_PTR result = state->WndProcDialog(hwnd, msg, wparam, lparam);

			//By default, Windows does not destroy modeless dialogs in response to a
			//WM_CLOSE message. We change that default here, and destroy the dialog window
			//on a WM_CLOSE message, to make the behaviour consistent with a normal
			//window. Note that we must always destroy any window which was created for us
			//by the UI manager by calling the CloseWindow method, so that it is able to
			//correctly remove this window from the interface, and perform any necessary
			//cleanup.
			if(result == FALSE)
			{
				state->uiManager.CloseWindow(*state, state->viewPresenter, hwnd);
			}

			//Since we've handled this message ourselves, return true. We also need to
			//abort right here regardless, as if the call to DestroyWindow has succeeded,
			//state is no longer valid, as the object would have been destroyed.
			return TRUE;
		}
		break;
	case WM_DESTROY:
		if(state != 0)
		{
			//Notify the UI manager that the window has been destroyed. Note that we need
			//to do this for any window that the UI manager created for us.
			state->uiManager.NotifyWindowDestroyed(*state, state->viewPresenter, hwnd);

			//Pass this message on to the member window procedure function
			INT_PTR result = state->WndProcDialog(hwnd, msg, wparam, lparam);

			//Discard the object pointer
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);

			//Notify the view presenter that the view has closed
			state->viewPresenter.NotifyViewClosed(state);

			//Return the result from processing the message
			return result;
		}
		break;
	}

	//Pass this message on to the member window procedure function
	INT_PTR result = FALSE;
	if(state != 0)
	{
		result = state->WndProcDialog(hwnd, msg, wparam, lparam);
	}
	return result;
}

//----------------------------------------------------------------------------------------
LRESULT CALLBACK ViewBase::WndProcWindowInternal(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//Obtain the object pointer
	ViewBase* state = (ViewBase*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	//Process the message
	switch(msg)
	{
	case WM_CREATE:
		//Set the object pointer
		state = (ViewBase*)((CREATESTRUCT*)lparam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(state));

		//Pass this message on to the member window procedure function
		if(state != 0)
		{
			return state->WndProcWindow(hwnd, msg, wparam, lparam);
		}
		break;
	case WM_DESTROY:
		if(state != 0)
		{
			//Pass this message on to the member window procedure function
			LRESULT result = state->WndProcWindow(hwnd, msg, wparam, lparam);

			//Discard the object pointer
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);

			//Notify the view presenter that the view has closed
			state->viewPresenter.NotifyViewClosed(state);

			//Return the result from processing the message
			return result;
		}
		break;
	}

	//Pass this message on to the member window procedure function
	if(state != 0)
	{
		return state->WndProcWindow(hwnd, msg, wparam, lparam);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}