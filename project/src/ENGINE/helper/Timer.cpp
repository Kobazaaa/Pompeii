// -- Standard Library --
#include <iostream>

// -- Pompeii Includes --
#include "Timer.h"
#include "ConsoleTextSettings.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Timer	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Behavioural
//--------------------------------------------------
void pompeii::Timer::Start()
{
	m_LastTimePoint = std::chrono::high_resolution_clock::now();
	m_CurrentTimePoint = std::chrono::high_resolution_clock::now();
	m_DeltaTimeSeconds = 0;
	m_SleepTimeSeconds = 0;
}
void pompeii::Timer::Update()
{
	++m_Ticks;

	m_CurrentTimePoint = std::chrono::high_resolution_clock::now();
	m_DeltaTimeSeconds = std::chrono::duration<float>(m_CurrentTimePoint - m_LastTimePoint).count();
	m_LastTimePoint = m_CurrentTimePoint;
	m_TotalTimeSeconds += m_DeltaTimeSeconds;
}

void pompeii::Timer::StartBenchmark()
{
	m_BenchmarkStart = std::chrono::high_resolution_clock::now();
}
float pompeii::Timer::EndBenchmark(bool printResults, const std::string& txt)
{
	m_BenchmarkEnd = std::chrono::high_resolution_clock::now();
	const float delta = std::chrono::duration<float, std::milli>(m_BenchmarkEnd - m_BenchmarkStart).count();

	if (printResults)
	{
		std::cout << DARK_YELLOW_TXT << txt << delta << "ms." << RESET_TXT << "\n";
	}

	return delta;
}


//--------------------------------------------------
//    Accessors
//--------------------------------------------------
float pompeii::Timer::GetDeltaSeconds()
{
	return m_DeltaTimeSeconds;
}
float pompeii::Timer::GetTotalTimeSeconds()
{
	return m_TotalTimeSeconds;
}
float pompeii::Timer::TargetFPS()
{
	return TARGET_FPS;
}
std::chrono::nanoseconds pompeii::Timer::SleepDurationNanoSeconds()
{
	constexpr auto msPerFrame = std::chrono::milliseconds(static_cast<int>(1'000.f / TARGET_FPS));
	const std::chrono::nanoseconds sleep = (m_CurrentTimePoint + msPerFrame - std::chrono::high_resolution_clock::now());

	return sleep;
}
