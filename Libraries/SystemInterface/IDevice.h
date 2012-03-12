#ifndef __IDEVICE_H__
#define __IDEVICE_H__
#include <string>
#include <list>
#include "IBusInterface.h"
#include "IMenuSubmenu.h"
#include "ImageInterface/ImageInterface.pkg"
class IHeirarchicalStorageNode;
class Data;
class IDeviceContext;

//##TODO## Add versioning to this interface, and all other system interfaces, so we can
//extend them in the future, without breaking compatibility with existing plugins.
class IDevice
{
public:
	//Enumerations
	enum UpdateMethod
	{
		UPDATEMETHOD_NONE,
		UPDATEMETHOD_STEP,
		UPDATEMETHOD_TIMESLICE
	};

	//Typedefs
	typedef void* AssemblyHandle;

	//Constructors
	virtual ~IDevice() = 0 {}

	//Initialization functions
	virtual bool BindToDeviceContext(IDeviceContext* adeviceContext) = 0;
	virtual bool Construct(IHeirarchicalStorageNode& node) = 0;
	virtual bool BuildDevice() = 0;
	virtual bool ValidateDevice() = 0;
	virtual void Initialize() = 0;

	//Reference functions
	inline bool AddReference(const std::wstring& referenceName, IBusInterface* target);
	virtual bool AddReference(const wchar_t* referenceName, IBusInterface* target) = 0;
	inline bool AddReference(const std::wstring& referenceName, IDevice* target);
	virtual bool AddReference(const wchar_t* referenceName, IDevice* target) = 0;
	//##TODO## Implement these functions as part of the new clock system
//	inline bool AddReference(const std::wstring& referenceName, IClockSource* target);
//	virtual bool AddReference(const wchar_t* referenceName, IClockSource* target) = 0;
	virtual bool RemoveReference(IBusInterface* target) = 0;
	virtual bool RemoveReference(IDevice* target) = 0;

	//Device context functions
	virtual IDeviceContext* GetDeviceContext() const = 0;
	virtual double GetCurrentTimesliceProgress() const = 0;

	//Execute functions
	virtual void BeginExecution() = 0;
	virtual void SuspendExecution() = 0;
	virtual void NotifyUpcomingTimeslice(double nanoseconds) = 0;
	virtual double ExecuteStep() = 0;
	virtual void ExecuteTimeslice(double nanoseconds) = 0;
	virtual double GetNextTimingPointInDeviceTime() const = 0;
	virtual void ExecuteRollback() = 0;
	virtual void ExecuteCommit() = 0;
	virtual UpdateMethod GetUpdateMethod() const = 0;
	virtual bool SendNotifyUpcomingTimeslice() const = 0;
	virtual void NotifyBeforeExecuteCalled() = 0;
	virtual bool SendNotifyBeforeExecuteCalled() const = 0;
	virtual void NotifyAfterExecuteCalled() = 0;
	virtual bool SendNotifyAfterExecuteCalled() const = 0;

	//Name functions
	inline std::wstring GetDeviceClassName() const;
	inline std::wstring GetDeviceInstanceName() const;
	inline std::wstring GetModuleDisplayName() const;
	virtual unsigned int GetDeviceModuleID() const = 0;

	//Savestate functions
	virtual bool GetScreenshot(IImage& image) const = 0;
	virtual void LoadState(IHeirarchicalStorageNode& node) = 0;
	virtual void GetState(IHeirarchicalStorageNode& node) const = 0;
	//##TODO## Implement saving and loading of the debugger state
	virtual void SaveDebuggerState(IHeirarchicalStorageNode& node) const = 0;
	virtual void LoadDebuggerState(IHeirarchicalStorageNode& node) = 0;

	//CE line state functions
	inline unsigned int GetCELineID(const std::wstring& lineName, bool inputLine) const;
	virtual unsigned int GetCELineID(const wchar_t* lineName, bool inputLine) const = 0;
	virtual void SetCELineInput(unsigned int lineID, bool lineMapped, unsigned int lineStartBitNumber) = 0;
	virtual void SetCELineOutput(unsigned int lineID, bool lineMapped, unsigned int lineStartBitNumber) = 0;
	virtual unsigned int CalculateCELineStateMemory(unsigned int location, const Data& data, unsigned int currentCELineState, const IBusInterface* sourceBusInterface, IDeviceContext* caller, double accessTime) const = 0;
	virtual unsigned int CalculateCELineStateMemoryTransparent(unsigned int location, const Data& data, unsigned int currentCELineState, const IBusInterface* sourceBusInterface, IDeviceContext* caller) const = 0;
	virtual unsigned int CalculateCELineStatePort(unsigned int location, const Data& data, unsigned int currentCELineState, const IBusInterface* sourceBusInterface, IDeviceContext* caller, double accessTime) const = 0;
	virtual unsigned int CalculateCELineStatePortTransparent(unsigned int location, const Data& data, unsigned int currentCELineState, const IBusInterface* sourceBusInterface, IDeviceContext* caller) const = 0;

	//Memory functions
	virtual IBusInterface::AccessResult ReadInterface(unsigned int interfaceNumber, unsigned int location, Data& data, IDeviceContext* caller, double accessTime) = 0;
	virtual IBusInterface::AccessResult WriteInterface(unsigned int interfaceNumber, unsigned int location, const Data& data, IDeviceContext* caller, double accessTime) = 0;
	virtual void TransparentReadInterface(unsigned int interfaceNumber, unsigned int location, Data& data, IDeviceContext* caller) = 0;
	virtual void TransparentWriteInterface(unsigned int interfaceNumber, unsigned int location, const Data& data, IDeviceContext* caller) = 0;

	//Port functions
	virtual IBusInterface::AccessResult ReadPort(unsigned int interfaceNumber, unsigned int location, Data& data, IDeviceContext* caller, double accessTime) = 0;
	virtual IBusInterface::AccessResult WritePort(unsigned int interfaceNumber, unsigned int location, const Data& data, IDeviceContext* caller, double accessTime) = 0;
	virtual void TransparentReadPort(unsigned int interfaceNumber, unsigned int location, Data& data, IDeviceContext* caller) = 0;
	virtual void TransparentWritePort(unsigned int interfaceNumber, unsigned int location, const Data& data, IDeviceContext* caller) = 0;

	//Line functions
	inline unsigned int GetLineID(const std::wstring& lineName) const;
	virtual unsigned int GetLineID(const wchar_t* lineName) const = 0;
	virtual const wchar_t* GetLineName(unsigned int lineID) const = 0;
	virtual unsigned int GetLineWidth(unsigned int lineID) const = 0;
	virtual void SetLineState(unsigned int targetLine, const Data& lineData, IDeviceContext* caller, double accessTime) = 0;

	//Clock source functions
	//##FIX## The devices need a way to detect, and validate, that a clock source has been
	//mapped. I would suggest that since 0 is an invalid clock rate, initialize clock
	//sources to 0 internally in each device, and if a required clock rate is 0, fail
	//validation.
	//##FIX## Devices need to be able to programmatically alter clock sources during
	//execution. This requires an actual interface back to the clock source. Should this
	//be done through the BusInterface, or directly via a reference to the clock source? I
	//would suggest it should be done by a reference to the clock source. Lines only go
	//through the bus because a single line may be mapped to multiple devices. We still
	//need the kind of interface shown below for propagating clock source rate changes
	//however, since in this case the clock source needs to be able to notify its
	//subscribers of changes to the clock rate.
	inline unsigned int GetClockSourceID(const std::wstring& lineName) const;
	virtual unsigned int GetClockSourceID(const wchar_t* lineName) const = 0;
	virtual const wchar_t* GetClockSourceName(unsigned int lineID) const = 0;
	virtual void SetClockSourceRate(unsigned int clockInput, double clockRate, IDeviceContext* caller, double accessTime) = 0;

	//Input functions
	inline unsigned int GetKeyCodeID(const std::wstring& keyCodeName) const;
	virtual unsigned int GetKeyCodeID(const wchar_t* keyCodeName) const = 0;
	virtual void HandleInputKeyDown(unsigned int keyCode) = 0;
	virtual void HandleInputKeyUp(unsigned int keyCode) = 0;

	//Window functions
	virtual void SetAssemblyHandle(AssemblyHandle aassemblyHandle) = 0;
	virtual AssemblyHandle GetAssemblyHandle() const = 0;
	virtual void AddGlobalDebugMenuItems(IMenuSegment& menuSegment, IViewModelLauncher& viewModelLauncher) = 0;
	virtual void AddDebugMenuItems(IMenuSegment& menuSegment, IViewModelLauncher& viewModelLauncher) = 0;
	inline void RestoreViewModelState(const std::wstring& menuHandlerName, int viewModelID, IHeirarchicalStorageNode& node, int xpos, int ypos, int width, int height, IViewModelLauncher& viewModelLauncher);
	virtual void RestoreViewModelState(const wchar_t* menuHandlerName, int viewModelID, IHeirarchicalStorageNode& node, int xpos, int ypos, int width, int height, IViewModelLauncher& viewModelLauncher) = 0;
	inline void OpenViewModel(const std::wstring& menuHandlerName, int viewModelID, IViewModelLauncher& viewModelLauncher);
	virtual void OpenViewModel(const wchar_t* menuHandlerName, int viewModelID, IViewModelLauncher& viewModelLauncher) = 0;

protected:
	//Name functions
	virtual const wchar_t* GetDeviceClassNameInternal() const = 0;
	virtual const wchar_t* GetDeviceInstanceNameInternal() const = 0;
	virtual const wchar_t* GetModuleDisplayNameInternal() const = 0;
};

#include "IDevice.inl"
#endif