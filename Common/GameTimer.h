#ifndef GAME_TIMER_H
#define GAME_TIMER_H

class GameTimer
{
public:
	GameTimer();

	float DeltaTime() const;	// 초 단위
	float TotalTime() const;	// 초 단위

	void Reset();				// 메시지 루프 이후 호출
	void Start();				
	void Stop();				
	void Tick();				// 매 프레임마다 호출

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};

#endif /* GAME_TIMER_H */