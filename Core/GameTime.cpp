#include "GameTime.h"
#include <windows.h>

GameTime::GameTime() : mSencondsPerCount(0.0f), mDeltaTime(-1.0f), mCurrentCount(0), mPreviewCount(0)
{
	__int64 countsPerSencond;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSencond); //每秒多少次
	mSencondsPerCount = 1.0 / (double)countsPerSencond;  //每次消耗的时间

	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	mStartCount = currentTime;
	mPreviewCount = currentTime;
}

GameTime::~GameTime()
{

}

void GameTime::Tick()
{
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	mCurrentCount = currentTime;
	mDeltaTime = (mCurrentCount - mPreviewCount) * mSencondsPerCount;

	mPreviewCount = currentTime;
}

double GameTime::GetTotalTime() const
{
	return (mCurrentCount - mStartCount) * mSencondsPerCount;
}

double GameTime::GetDeltaTime() const
{
	return mDeltaTime;
}
