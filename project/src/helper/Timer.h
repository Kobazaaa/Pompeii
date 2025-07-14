#ifndef TIMER_H
#define TIMER_H

// -- Time --
#include <chrono>
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;


// -- Class --
namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Timer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Timer final
	{
	public:
		//--------------------------------------------------
		//    Constructors and Destructors
		//--------------------------------------------------
		Timer() = delete;
		~Timer() = delete;

		Timer(const Timer& other) = delete;
		Timer(Timer&& other) = delete;
		Timer& operator=(const Timer& other) = delete;
		Timer& operator=(Timer&& other) = delete;


		//--------------------------------------------------
		//    Behavioural
		//--------------------------------------------------
		static void Start();
		static void Update();

		static void StartBenchmark();
		static float EndBenchmark(bool printResults, const std::string& txt = "");

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		[[nodiscard]] static float GetDeltaSeconds();
		[[nodiscard]] static float GetTotalTimeSeconds();
		[[nodiscard]] static float TargetFPS();
		[[nodiscard]] static std::chrono::nanoseconds SleepDurationNanoSeconds();

	private:
		//--------------------------------------------------
		//    Time Variables
		//--------------------------------------------------
		// Immutable
		static constexpr float	TARGET_FPS					{ 60.f };

		// Mutable
		inline static float				m_TotalTimeSeconds	{ };
		inline static float				m_DeltaTimeSeconds	{ };
		inline static float				m_SleepTimeSeconds	{ };

		inline static int				m_Ticks				{ };

		inline static TimePoint			m_CurrentTimePoint	{ };
		inline static TimePoint			m_LastTimePoint		{ };

		inline static TimePoint			m_BenchmarkStart	{ };
		inline static TimePoint			m_BenchmarkEnd		{ };
	};
}

#endif // TIMER_H
