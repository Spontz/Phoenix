// Logger.h
// Spontz Demogroup

#pragma once

#include <deque>
#include <cstdint>
#include <format>
#include <mutex>
#include <string>
#include <vector>

// TODO: Use a single Logger.h file, instead of "Logger.h + LoggerDeclarations.h" files

namespace Phoenix {

	enum class LogLevel {
		none = 0,
		high = 1,
		med = 2,
		low = 3
	};

	class Logger final
	{
	public:
		struct RecentEntry final
		{
			uint64_t sequence = 0;
			std::string severity;
			std::string message;
		};

		class ScopedIndent final
		{
		public:
			ScopedIndent();
			~ScopedIndent();
		};

	public:
		Logger() = delete;

	public:
		template <class... _Types>
		static void info(LogLevel level, const std::string_view msg, const _Types&... args);

		template <class... _Types>
		static void sendEditor(const std::string_view msg, const _Types&... args);

		template <class... _Types>
		static void error(const std::string_view msg, const _Types&... args);

	public:
		static void setLogLevel(LogLevel level);
		static void openLogFile();
		static void closeLogFile();
		static std::vector<RecentEntry> getRecentEntries();

	private:
		static std::string formatMsg(const std::string_view src, const std::string_view Message);
		static void rememberRecent(const std::string_view severity, const std::string_view message);

	private:
		static LogLevel kLogLevel;
		static std::ofstream kOutputStream;
		static uint64_t kNextRecentSequence;
		static std::mutex kRecentMutex;
		static std::deque<RecentEntry> kRecentEntries;

	};

}
