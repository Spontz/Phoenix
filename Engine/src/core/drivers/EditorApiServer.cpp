// EditorApiServer.cpp
// Spontz Demogroup

#include "core/drivers/EditorApiServer.h"

#include "core/DemoKernel.h"
#include "core/utils/Logger.h"

#include <uwebsockets/App.h>

#include <cmath>
#include <format>

namespace Phoenix {

	namespace {
		EditorApiServer* kpEditorApiServer = nullptr;
		constexpr auto kRuntimeTopic = "runtime";
		constexpr auto kRuntimePublishInterval = std::chrono::milliseconds(33);

		struct WebSocketData {
		};
	}

	EditorApiServer& EditorApiServer::getInstance()
	{
		if (!kpEditorApiServer)
			kpEditorApiServer = new EditorApiServer();
		return *kpEditorApiServer;
	}

	void EditorApiServer::release()
	{
		delete kpEditorApiServer;
		kpEditorApiServer = nullptr;
	}

	EditorApiServer::EditorApiServer()
		:
		m_initialized(false),
		m_running(false),
		m_listening(false),
		m_loop(nullptr),
		m_app(nullptr),
		m_listenSocket(nullptr),
		m_lastRuntimePublish(std::chrono::steady_clock::now()),
		m_port(29100)
	{
	}

	EditorApiServer::~EditorApiServer()
	{
		shutdown();
	}

	void EditorApiServer::init()
	{
		bool expected = false;
		if (!m_initialized.compare_exchange_strong(expected, true))
			return;

		m_running = true;
		m_thread = std::thread(&EditorApiServer::runServer, this);
		Logger::info(LogLevel::high, "Editor API: starting HTTP/WebSocket server on port {}", m_port);
	}

	void EditorApiServer::update()
	{
		if (!m_initialized)
			return;

		processCommands();
		publishRuntimeState();
	}

	void EditorApiServer::shutdown()
	{
		if (!m_initialized.exchange(false))
			return;

		m_running = false;

		if (m_loop) {
			m_loop->defer([this]() {
				if (m_listenSocket) {
					us_listen_socket_close(0, static_cast<us_listen_socket_t*>(m_listenSocket));
					m_listenSocket = nullptr;
				}
				if (m_app)
					m_app->close();
			});
		}

		if (m_thread.joinable())
			m_thread.join();

		std::lock_guard lock(m_commandMutex);
		std::queue<Command> empty;
		m_commands.swap(empty);

		Logger::info(LogLevel::high, "Editor API: stopped");
	}

	void EditorApiServer::runServer()
	{
		uWS::App app;
		m_app = &app;
		m_loop = uWS::Loop::get();

		app.get("/api/health", [this](auto* response, auto*) {
			response
				->writeHeader("Access-Control-Allow-Origin", "*")
				->writeHeader("Content-Type", "application/json")
				->end(std::format(
					"{{\"status\":\"ok\",\"service\":\"phoenix-editor-api\",\"port\":{}}}",
					m_port
				));
		});

		app.options("/*", [](auto* response, auto*) {
			response
				->writeHeader("Access-Control-Allow-Origin", "*")
				->writeHeader("Access-Control-Allow-Methods", "GET, OPTIONS")
				->writeHeader("Access-Control-Allow-Headers", "Content-Type")
				->end();
		});

		app.ws<WebSocketData>("/ws", {
			.open = [](auto* ws) {
				ws->subscribe(kRuntimeTopic);
				Logger::info(LogLevel::high, "Editor API: Cacablu WebSocket connected");
			},
			.message = [this](auto*, std::string_view message, uWS::OpCode opCode) {
				if (opCode != uWS::OpCode::TEXT) {
					enqueueError("invalid-opcode", "Only text WebSocket messages are supported");
					return;
				}
				enqueueMessage(message);
			},
			.close = [](auto*, int, std::string_view) {
				Logger::info(LogLevel::high, "Editor API: Cacablu WebSocket disconnected");
			}
		});

		app.listen("127.0.0.1", m_port, [this](auto* token) {
			m_listenSocket = token;
			m_listening = token != nullptr;

			if (token)
				Logger::info(LogLevel::high, "Editor API: listening at ws://127.0.0.1:{}/ws", m_port);
			else
				Logger::error("Editor API: could not listen on port {}", m_port);
		});

		app.run();

		m_app = nullptr;
		m_loop = nullptr;
		m_listening = false;
	}

	void EditorApiServer::enqueueMessage(std::string_view message)
	{
		std::string type;
		if (!extractMessageType(message, type)) {
			enqueueError("invalid-message", "Missing message type");
			return;
		}

		Command command{};
		if (type == "runtime.play") {
			command.type = CommandType::Play;
		}
		else if (type == "runtime.pause") {
			command.type = CommandType::Pause;
		}
		else if (type == "runtime.toggle") {
			command.type = CommandType::Toggle;
		}
		else if (type == "runtime.seek") {
			float time = 0;
			if (!extractNumber(message, "time", time) || !std::isfinite(time) || time < 0) {
				enqueueError("invalid-seek", "runtime.seek requires a non-negative finite time");
				return;
			}
			command.type = CommandType::Seek;
			command.time = time;
		}
		else {
			enqueueError("unsupported-message", std::format("Unsupported message type: {}", type));
			return;
		}

		std::lock_guard lock(m_commandMutex);
		m_commands.emplace(command);
	}

	void EditorApiServer::enqueueError(std::string_view code, std::string_view message)
	{
		if (!m_loop || !m_app)
			return;

		const std::string payload = buildErrorMessage(code, message);
		m_loop->defer([this, payload]() {
			if (m_app)
				m_app->publish(kRuntimeTopic, payload, uWS::OpCode::TEXT);
		});
	}

	void EditorApiServer::processCommands()
	{
		std::queue<Command> commands;
		{
			std::lock_guard lock(m_commandMutex);
			commands.swap(m_commands);
		}

		while (!commands.empty()) {
			const Command command = commands.front();
			commands.pop();

			switch (command.type) {
			case CommandType::Play:
				DEMO->playDemo();
				break;
			case CommandType::Pause:
				DEMO->pauseDemo();
				break;
			case CommandType::Toggle:
				if (DEMO->m_status & DemoStatus::PLAY)
					DEMO->pauseDemo();
				else
					DEMO->playDemo();
				break;
			case CommandType::Seek:
				DEMO->setCurrentTime(command.time);
				break;
			default:
				break;
			}
		}
	}

	void EditorApiServer::publishRuntimeState()
	{
		if (!m_listening || !m_loop || !m_app)
			return;

		const auto now = std::chrono::steady_clock::now();
		if (now - m_lastRuntimePublish < kRuntimePublishInterval)
			return;

		m_lastRuntimePublish = now;
		const std::string payload = buildRuntimeStateMessage();

		m_loop->defer([this, payload]() {
			if (m_app)
				m_app->publish(kRuntimeTopic, payload, uWS::OpCode::TEXT);
		});
	}

	std::string EditorApiServer::buildRuntimeStateMessage() const
	{
		const bool playing = (DEMO->m_status & DemoStatus::PLAY) != 0;
		return std::format(
			"{{\"type\":\"runtime.state\",\"time\":{:.6f},\"playing\":{},\"fps\":{:.3f},\"startTime\":{:.6f},\"endTime\":{:.6f}}}",
			DEMO->m_demoRunTime,
			playing ? "true" : "false",
			DEMO->m_fps,
			DEMO->m_demoStartTime,
			DEMO->m_demoEndTime
		);
	}

	std::string EditorApiServer::buildErrorMessage(std::string_view code, std::string_view message) const
	{
		return std::format(
			"{{\"type\":\"error\",\"code\":\"{}\",\"message\":\"{}\"}}",
			code,
			message
		);
	}

	bool EditorApiServer::extractMessageType(std::string_view message, std::string& type)
	{
		const std::string_view key = "\"type\"";
		size_t pos = message.find(key);
		if (pos == std::string_view::npos)
			return false;

		pos = message.find(':', pos + key.size());
		if (pos == std::string_view::npos)
			return false;

		pos = message.find('"', pos + 1);
		if (pos == std::string_view::npos)
			return false;

		const size_t end = message.find('"', pos + 1);
		if (end == std::string_view::npos)
			return false;

		type.assign(message.substr(pos + 1, end - pos - 1));
		return true;
	}

	bool EditorApiServer::extractNumber(std::string_view message, std::string_view key, float& value)
	{
		const std::string quotedKey = std::format("\"{}\"", key);
		size_t pos = message.find(quotedKey);
		if (pos == std::string_view::npos)
			return false;

		pos = message.find(':', pos + quotedKey.size());
		if (pos == std::string_view::npos)
			return false;

		const size_t start = message.find_first_of("-0123456789", pos + 1);
		if (start == std::string_view::npos)
			return false;

		const size_t end = message.find_first_not_of("-0123456789.eE+", start);
		const std::string raw(message.substr(start, end == std::string_view::npos ? end : end - start));

		try {
			value = std::stof(raw);
			return true;
		}
		catch (...) {
			return false;
		}
	}
}
