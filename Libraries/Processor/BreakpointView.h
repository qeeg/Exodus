#include "Processor.h"
#ifndef __PROCESSOR_BREAKPOINTVIEW_H__
#define __PROCESSOR_BREAKPOINTVIEW_H__
#include "WindowFunctions/WindowFunctions.pkg"
#include "SystemInterface/SystemInterface.pkg"
#include "Breakpoint.h"

class Processor::BreakpointView :public ViewBase
{
public:
	//Constructors
	BreakpointView(Processor* adevice);

protected:
	//Member window procedure
	virtual INT_PTR WndProcDialog(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	//Event handlers
	INT_PTR msgWM_INITDIALOG(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT_PTR msgWM_CLOSE(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT_PTR msgWM_TIMER(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT_PTR msgWM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam);

	//Breakpoint functions
	void UpdateBreakpoint(HWND hwnd, const Breakpoint& breakpoint, unsigned int addressCharWidth);

private:
	Processor* device;
	bool initializedDialog;
	std::wstring previousText;
	unsigned int currentControlFocus;
	int breakpointListIndex;
	Breakpoint breakpoint;
};

#endif