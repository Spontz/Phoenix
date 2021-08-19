#pragma once

#include "core/utils/LoggerDeclarations.h"
#include "core/DemoKernel.h"

namespace Phoenix {

	template <class... _Types>
	void Logger::info(LogLevel level, const std::string_view msg, const _Types&... args)
	{
		if (!DEMO->m_debug || kLogLevel < level)
			return;

		// Output to file
		const auto s = formatMsg("Info", vformat(msg, std::make_format_args(args...)));
		kOutputStream << s;

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		if (level <= LogLevel::high)
			OutputDebugStringA(s.c_str());
#endif
	}

	template <class... _Types>
	void Logger::sendEditor(const std::string_view msg, const _Types&... args)
	{
		// We send the message only if we are in debug mode and slave mode
		if (!DEMO->m_debug || !DEMO->m_slaveMode)
			return;

		const auto s = formatMsg("INFO::Info", vformat(msg, std::make_format_args(args...)));

		// Output to editor
		NetDriver::getInstance().sendMessage(s);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(s.c_str());
#endif
	}

	template <class... _Types>
	void Logger::error(const std::string_view msg, const _Types&... args)
	{
		if (!DEMO->m_debug)
			return;

		const auto s = formatMsg("Error", vformat(msg, std::make_format_args(args...)));

		// Output to file
		kOutputStream << s;

		// Output to screen log
		GLDRV->guiAddLog(s);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(s.c_str());
#endif

		// Output to demo editor
		NetDriver::getInstance().sendMessage("ERROR::" + s);
	}

}
