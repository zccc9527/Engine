#pragma once
class GameTime
{
public:
	GameTime();
	~GameTime();

	void Tick();
	double GetTotalTime() const;
	double GetDeltaTime() const;
private:
	double mSencondsPerCount; //每次计数花费的时间
	double mDeltaTime; //每帧的时间,当前帧次数减去上一帧次数乘以每次计数花费的时间
	__int64 mStartCount; //开始运行时的计数
	__int64 mCurrentCount; //当前计数
	__int64 mPreviewCount; //上一帧计数
};

