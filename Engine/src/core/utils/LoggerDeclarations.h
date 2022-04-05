// Logger.h
// Spontz Demogroup

#pragma once

#include <format>

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

	private:
		static std::string formatMsg(const std::string_view src, const std::string_view Message);

	private:
		static LogLevel kLogLevel;
		static std::ofstream kOutputStream;

	};

}
