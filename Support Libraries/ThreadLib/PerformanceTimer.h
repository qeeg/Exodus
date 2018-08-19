#ifndef __PERFORMANCETIMER_H__
#define __PERFORMANCETIMER_H__
#include "WindowsSupport/WindowsSupport.pkg"

class PerformanceTimer
{
public:
	// Constructors
	inline PerformanceTimer();

	// Synchronization functions
	inline void Reset();
	inline void Sync(double targetExecutionTime, bool enableSync = true, bool outputTimerDebug = false, double executeAheadTolerance = 0.001);

private:
	LARGE_INTEGER _counterFrequency;
	LARGE_INTEGER _executionTimeStart;
	LONGLONG _executionTimeAhead;
};

#include "PerformanceTimer.inl"
#endif
