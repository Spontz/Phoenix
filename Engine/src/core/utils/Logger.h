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
		static void info(LogLevel level, std::string_view Message, ...);
		static void sendEditor(std::string_view Message, ...);
		static void error(std::string_view Message, ...);
		static void openLogFile();
		static void closeLogFile();
	};

}
