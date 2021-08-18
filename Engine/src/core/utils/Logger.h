// Logger.h
// Spontz Demogroup

#pragma once

#include <format>

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
		static void info(LogLevel level, const std::string_view msg, const _Types&... args) {
			info(level, vformat(msg, std::make_format_args(args...)));
		}

		template <class... _Types>
		static void sendEditor(const std::string_view msg, const _Types&... args) {
			sendEditor(vformat(msg, std::make_format_args(args...)));
		}

		template <class... _Types>
		static void error(const std::string_view msg, const _Types&... args) {
			error(vformat(msg, std::make_format_args(args...)));
		}

	public:
		static void info(LogLevel level, const std::string_view msg);
		static void sendEditor(const std::string_view msg);
		static void error(const std::string_view msg);
		static void setLogLevel(LogLevel level);
		static void openLogFile();
		static void closeLogFile();
	};

}
