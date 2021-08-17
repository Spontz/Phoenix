// Logger.h
// Spontz Demogroup

#pragma once

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
		static void setLogLevel(LogLevel level);
		static void info(LogLevel level, const char* pszMessage, ...);
		static void sendEditor(const char* pszMessage, ...);
		static void error(const char* pszMessage, ...);
		static void openLogFile();
		static void closeLogFile();
	};

}
