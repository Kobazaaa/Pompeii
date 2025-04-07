#include "Timer.h"
#include <chrono>

//--------------------------------------------------
//    Behavioural
//--------------------------------------------------
void pom::Timer::Start()
{
	m_LastTimePoint = std::chrono::high_resolution_clock::now();
	m_CurrentTimePoint = std::chrono::high_resolution_clock::now();
	m_DeltaTimeSeconds = 0;
	m_SleepTimeSeconds = 0;
}
void pom::Timer::Update()
{
	++m_Ticks;

	m_CurrentTimePoint = std::chrono::high_resolution_clock::now();
	m_DeltaTimeSeconds = std::chrono::duration<float>(m_CurrentTimePoint - m_LastTimePoint).count();
	m_LastTimePoint = m_CurrentTimePoint;
	m_TotalTimeSeconds += m_DeltaTimeSeconds;
}



//--------------------------------------------------
//    Accessors
//--------------------------------------------------
float pom::Timer::GetDeltaSeconds()
{
	return m_DeltaTimeSeconds;
}
float pom::Timer::GetTotalTimeSeconds()
{
	return m_TotalTimeSeconds;
}
float pom::Timer::TargetFPS()
{
	return TARGET_FPS;
}
std::chrono::nanoseconds pom::Timer::SleepDurationNanoSeconds()
{
	constexpr auto msPerFrame = std::chrono::milliseconds(static_cast<int>(1'000.f / TARGET_FPS));
	const std::chrono::nanoseconds sleep = (m_CurrentTimePoint + msPerFrame - std::chrono::high_resolution_clock::now());

	return sleep;
}
