// EditorApiServer.h
// Spontz Demogroup

#pragma once

#include <atomic>
#include <array>
#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

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
		void applyRemoteInputToImGui();
		bool isRemoteKeyPressed(int32_t key) const;

	private:
		enum class CommandType {
			Play,
			Pause,
			Toggle,
			Seek,
			MouseMove,
			MouseDown,
			MouseUp,
			MouseWheel,
			KeyDown,
			KeyUp,
			EnableWebRtc,
			DisableWebRtc
		};

		struct Command {
			CommandType type;
			float time;
			float x;
			float y;
			float deltaX;
			float deltaY;
			int32_t button;
			int32_t clientId;
			int32_t key;
			int32_t scancode;
			int32_t mods;
			bool repeat;
		};
		using RemoteInputEvent = Command;
		using WebRtcSignalSender = std::function<void(std::string_view type, std::string_view payload, std::string_view sdpMid)>;

		EditorApiServer();
		~EditorApiServer();

		EditorApiServer(const EditorApiServer&) = delete;
		EditorApiServer& operator=(const EditorApiServer&) = delete;

		void runServer();
		void enqueueMessage(std::string_view message);
		void enqueueError(std::string_view code, std::string_view message);
		void publishEvent(std::string_view payload);
		std::string handleWebRtcMessage(std::string_view message, int32_t clientId, const WebRtcSignalSender& signalSender);
		void clearRemoteKeys();
		void setRemoteKeyPressed(int32_t key, bool pressed);
		void processGraphicsConfigRequests();
		void processCommands();
		void processSectionReplaceRequests();
		void publishRuntimeState();
		std::string buildRuntimeStateMessage() const;
		std::string buildErrorMessage(std::string_view code, std::string_view message) const;
		std::string buildWebRtcOfferMessage(int32_t sessionId, std::string_view sdp) const;
		std::string buildWebRtcAnswerMessage(std::string_view sdp) const;
		std::string buildWebRtcCandidateMessage(std::string_view candidate, std::string_view sdpMid) const;
		std::string buildWebRtcStateMessage(std::string_view state) const;

	public:
		static bool extractMessageType(std::string_view message, std::string& type);
		static bool extractString(std::string_view message, std::string_view key, std::string& value);
		static bool extractNumber(std::string_view message, std::string_view key, float& value);
		static bool extractInteger(std::string_view message, std::string_view key, int32_t& value);
		static std::string escapeJson(std::string_view value);

	private:
		static constexpr size_t kRemoteKeyCount = 512;
		std::atomic_bool m_initialized;
		std::atomic_bool m_running;
		std::atomic_bool m_listening;
		std::thread m_thread;
		uWS::Loop* m_loop;
		uWS::App* m_app;
		void* m_listenSocket;
		std::mutex m_commandMutex;
		std::queue<Command> m_commands;
		std::unordered_set<int32_t> m_webRtcPreviewClients;
		bool m_remoteMouseActive;
		float m_remoteMouseX;
		float m_remoteMouseY;
		float m_remoteMouseWheelX;
		float m_remoteMouseWheelY;
		std::array<bool, 5> m_remoteMouseButtons;
		std::array<int32_t, 5> m_remoteMouseReleaseFrames;
		std::vector<RemoteInputEvent> m_remoteInputEvents;
		std::vector<RemoteInputEvent> m_nextFrameRemoteInputEvents;
		mutable std::mutex m_remoteKeyMutex;
		std::array<bool, kRemoteKeyCount> m_remoteKeys;
		std::chrono::steady_clock::time_point m_lastRuntimePublish;
		int32_t m_port;
	};
}
