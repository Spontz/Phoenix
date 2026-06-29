// EditorApiServer.h
// Spontz Demogroup

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace uWS {
	struct Loop;
	template <bool SSL>
	struct TemplatedApp;
	using App = TemplatedApp<false>;
}

namespace Phoenix {

	class EditorApiServer final {
	public:
		static EditorApiServer& getInstance();
		static void release();

		void init();
		void update();
		void shutdown();

	private:
		enum class CommandType {
			Play,
			Pause,
			Toggle,
			Seek
		};

		struct Command {
			CommandType type;
			float time;
		};

		EditorApiServer();
		~EditorApiServer();

		EditorApiServer(const EditorApiServer&) = delete;
		EditorApiServer& operator=(const EditorApiServer&) = delete;

		void runServer();
		void enqueueMessage(std::string_view message);
		void enqueueError(std::string_view code, std::string_view message);
		void processCommands();
		void publishRuntimeState();
		std::string buildRuntimeStateMessage() const;
		std::string buildErrorMessage(std::string_view code, std::string_view message) const;

		static bool extractMessageType(std::string_view message, std::string& type);
		static bool extractNumber(std::string_view message, std::string_view key, float& value);

	private:
		std::atomic_bool m_initialized;
		std::atomic_bool m_running;
		std::atomic_bool m_listening;
		std::thread m_thread;
		uWS::Loop* m_loop;
		uWS::App* m_app;
		void* m_listenSocket;
		std::mutex m_commandMutex;
		std::queue<Command> m_commands;
		std::chrono::steady_clock::time_point m_lastRuntimePublish;
		int32_t m_port;
	};
}
