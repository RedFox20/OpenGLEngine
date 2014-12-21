#include "Timer.h"
#include <Windows.h>


static double mDFrequency = [](){
	long long freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	return (double)freq;
}();
static float mFFrequency = (float)mDFrequency;

// initializes a stopped timer
Timer::Timer() : mStart(0), mStop(0)
{
}
// initializes a started timer
Timer::Timer(timerstart_t) : mStop(0)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mStart);
}
// starts the timer
void Timer::Start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mStart);
}
// stops the timer
void Timer::Stop()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mStop);
}
// gets the elapsed time in seconds
double Timer::Elapsed() const
{
	return double(mStop - mStart) / mDFrequency;
}
// stops the timer and gets elapsed time in seconds
double Timer::StopElapsed()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mStop);
	return double(mStop - mStart) / mDFrequency;
}

static Timer timer(true); // global start time
static double sample = 0.0;

double TimeSampler::NextSample()
{
	sample = timer.StopElapsed();
	timer.mStart = timer.mStop;
	return sample;
}

double TimeSampler::PrevSample()
{
	return sample;
}




SpareTime::SpareTime(float timeLeft) : mSpareTime(timeLeft)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mStart);
}

float SpareTime::TimeRemaining() const
{
	long long stop;
	QueryPerformanceCounter((LARGE_INTEGER*)&stop);
	float remaining = mSpareTime - (float(stop - mStart) / mFFrequency);
	return remaining > 0.0f ? remaining : 0.0f;
}

bool SpareTime::TimeRemaining(float bufferTime) const
{
	return TimeRemaining() > bufferTime;
}