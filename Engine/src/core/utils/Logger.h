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
		_NODISCARD static void info2(LogLevel level, const std::string_view fmt, const _Types&... args) {
			info(level, vformat(fmt, std::make_format_args(args...)));
		}

		template <class... _Types>
		_NODISCARD static void sendEditor2(const std::string_view fmt, const _Types&... args) {
			sendEditor(vformat(fmt, std::make_format_args(args...)));
		}

		template <class... _Types>
		_NODISCARD static void error2(const std::string_view fmt, const _Types&... args) {
			error(vformat(fmt, std::make_format_args(args...)));
		}

	public:
		static void setLogLevel(LogLevel level);
		static void info(LogLevel level, std::string_view Message, ...);
		static void sendEditor(std::string_view Message, ...);
		static void error(std::string_view Message, ...);
		static void openLogFile();
		static void closeLogFile();
	};

}
