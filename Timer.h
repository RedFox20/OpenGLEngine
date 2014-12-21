/**
 * Copyright (c) 2013 - Jorma Rebane
 * A cross-platform timer
 */
#pragma once
#ifndef TIMER_H
#define TIMER_H

typedef bool timerstart_t;
static const timerstart_t tstart = true;

struct Timer
{
	long long mStart;
	long long mStop;
	// initializes a stopped timer
	Timer();
	// initializes a started timer
	Timer(timerstart_t);
	// starts the timer
	void Start(); 
	// stops the timer
	void Stop();
	// gets the elapsed time in seconds
	double Elapsed() const;
	// stops the timer and gets elapsed time in seconds
	double StopElapsed();

	// measures time elapsed for executing the lambda or functor
	template<class T> inline static double Measure(T func)
	{
		Timer t(tstart);
		func();
		return t.StopElapsed();
	}
};




// timesample is used to calculate delta-time between frames
struct TimeSampler
{
	// gets the next delta-time sample
	static double NextSample();

	// gets the previous called sample value
	static double PrevSample();
};



/**
 * Spare time countdown timer used for exploiting limited spare time between tasks
 * Ex:
 *     You have 5ms before next vsync. Instead of Sleep(4), track time left with SpareTime
 *     and do additional computations.
 */
struct SpareTime
{
	float mSpareTime; // total spare time when we started
	long long mStart; // start timestamp

	/**
	 * Creates a new SpareTime countdown timer
	 */
	SpareTime(float timeLeft);

	/**
	 * @return Time remaining until spare time is over. 0.0f if no more spare time
	 */
	float TimeRemaining() const;

	/**
	 * @return TRUE if TimeRemaining() > bufferTime
	 */
	bool TimeRemaining(float bufferTime) const;
};


#endif // TIMER_H