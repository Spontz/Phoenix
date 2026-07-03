// EditorApiServer.cpp
// Spontz Demogroup

#include "core/drivers/EditorApiServer.h"

#include "core/DemoKernel.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"
#include "core/streaming/FramebufferStreamer.h"
#include "core/utils/Logger.h"
#include "libs.h"

#include <uwebsockets/App.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <atomic>
#include <unordered_map>
#include <vector>

namespace Phoenix {

	namespace {
		EditorApiServer* kpEditorApiServer = nullptr;
		constexpr auto kRuntimeTopic = "runtime";
		constexpr auto kRuntimePublishInterval = std::chrono::milliseconds(33);
		namespace fs = std::filesystem;

		struct WebSocketData {
			int32_t clientId = 0;
			bool webRtcPreviewRequested = false;
		};

		std::atomic_int32_t kNextWebSocketClientId = 1;

		struct AssetPath {
			std::string relative;
			fs::path full;
		};

		struct IncomingSection {
			std::string id;
			std::string type;
			float startTime = 0.0f;
			float endTime = 0.0f;
			bool enabled = true;
			int32_t layer = 0;
			std::string srcBlending;
			std::string dstBlending;
			std::string blendingEQ;
			std::string script;
			std::string content;
		};

		struct SectionReplaceResult {
			std::string status;
			std::string body;
			std::string eventPayload;
		};

		enum class SectionRequestOperation {
			ReplaceAll,
			UpdateOne,
			DeleteMany
		};

		struct SectionReplaceRequest {
			std::string requestId;
			std::vector<IncomingSection> sections;
			std::vector<std::string> sectionIds;
			SectionRequestOperation operation = SectionRequestOperation::ReplaceAll;
			SectionReplaceResult result;
			bool done = false;
			std::mutex mutex;
			std::condition_variable condition;
		};

		std::mutex kSectionReplaceQueueMutex;
		std::queue<std::shared_ptr<SectionReplaceRequest>> kSectionReplaceQueue;

		struct SectionSyncStatus {
			std::string requestId;
			std::string operation;
			std::string phase;
			std::string message;
			size_t current = 0;
			size_t total = 0;
			size_t loaded = 0;
			size_t failed = 0;
			bool done = false;
		};

		std::mutex kSectionSyncStatusMutex;
		std::unordered_map<std::string, SectionSyncStatus> kSectionSyncStatuses;

		template <typename Response>
		void writeCors(Response* response)
		{
			response
				->writeHeader("Access-Control-Allow-Origin", "*")
				->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
				->writeHeader("Access-Control-Allow-Headers", "Content-Type")
				->writeHeader("Access-Control-Allow-Private-Network", "true")
				->writeHeader("Content-Type", "application/json");
		}

		template <typename Response>
		void sendJson(Response* response, std::string_view status, std::string_view body)
		{
			writeCors(response);
			response->writeStatus(status)->end(body);
		}

		std::string escapeJsonValue(std::string_view value)
		{
			std::string escaped;
			escaped.reserve(value.size());
			for (const char ch : value) {
				switch (ch) {
				case '\\': escaped += "\\\\"; break;
				case '"': escaped += "\\\""; break;
				case '\n': escaped += "\\n"; break;
				case '\r': escaped += "\\r"; break;
				case '\t': escaped += "\\t"; break;
				default: escaped.push_back(ch); break;
				}
			}
			return escaped;
		}

		std::string buildAssetError(std::string_view code, std::string_view message, std::string_view requestId = {})
		{
			return std::format(
				"{{\"requestId\":\"{}\",\"ok\":false,\"code\":\"{}\",\"message\":\"{}\"}}",
				escapeJsonValue(requestId),
				escapeJsonValue(code),
				escapeJsonValue(message)
			);
		}

		void updateSectionSyncStatus(
			std::string_view requestId,
			std::string_view operation,
			std::string_view phase,
			size_t current,
			size_t total,
			size_t loaded,
			size_t failed,
			bool done,
			std::string_view message)
		{
			if (requestId.empty())
				return;

			std::lock_guard lock(kSectionSyncStatusMutex);
			kSectionSyncStatuses[std::string(requestId)] = SectionSyncStatus{
				.requestId = std::string(requestId),
				.operation = std::string(operation),
				.phase = std::string(phase),
				.message = std::string(message),
				.current = current,
				.total = total,
				.loaded = loaded,
				.failed = failed,
				.done = done
			};
		}

		std::string buildSectionSyncStatusJson(std::string_view requestId)
		{
			std::lock_guard lock(kSectionSyncStatusMutex);
			const auto it = kSectionSyncStatuses.find(std::string(requestId));
			if (it == kSectionSyncStatuses.end())
				return buildAssetError("section-sync-status-not-found", "Section sync status was not found", requestId);

			const auto& status = it->second;
			return std::format(
				"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"{}\",\"phase\":\"{}\",\"current\":{},\"total\":{},\"loaded\":{},\"failed\":{},\"done\":{},\"message\":\"{}\"}}",
				escapeJsonValue(status.requestId),
				escapeJsonValue(status.operation),
				escapeJsonValue(status.phase),
				status.current,
				status.total,
				status.loaded,
				status.failed,
				status.done ? "true" : "false",
				escapeJsonValue(status.message)
			);
		}

		std::string normalizeAssetPath(std::string_view rawPath)
		{
			std::string path(rawPath);
			std::replace(path.begin(), path.end(), '\\', '/');

			while (!path.empty() && path.front() == '/')
				path.erase(path.begin());

			std::vector<std::string> parts;
			std::stringstream stream(path);
			std::string part;
			while (std::getline(stream, part, '/')) {
				if (part.empty() || part == ".")
					continue;
				if (part == ".." || part.find(':') != std::string::npos)
					return {};
				parts.push_back(part);
			}

			if (parts.empty() || (parts[0] != "pool" && parts[0] != "resources"))
				return {};

			std::string normalized;
			for (size_t i = 0; i < parts.size(); ++i) {
				if (i > 0)
					normalized += '/';
				normalized += parts[i];
			}
			return normalized;
		}

		std::optional<AssetPath> resolveAssetPath(std::string_view rawPath)
		{
			const std::string normalized = normalizeAssetPath(rawPath);
			if (normalized.empty())
				return std::nullopt;

			fs::path fullPath = fs::path(DEMO->m_dataFolder) / fs::path(normalized);
			return AssetPath{
				.relative = normalized,
				.full = fullPath.lexically_normal()
			};
		}

		std::string bytesToHex(uint32_t value)
		{
			constexpr char hex[] = "0123456789abcdef";
			std::string out(8, '0');
			for (int i = 7; i >= 0; --i) {
				out[static_cast<size_t>(i)] = hex[value & 0x0f];
				value >>= 4;
			}
			return out;
		}

		std::string hashFile(const fs::path& path)
		{
			std::ifstream file(path, std::ios::binary);
			if (!file)
				return {};

			uint32_t hash = 0x811c9dc5u;
			char buffer[4096];
			while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
				for (std::streamsize i = 0; i < file.gcount(); ++i) {
					hash ^= static_cast<uint8_t>(buffer[i]);
					hash *= 0x01000193u;
				}
			}
			return "fnv1a:" + bytesToHex(hash);
		}

		std::string hashString(std::string_view value)
		{
			uint32_t hash = 0x811c9dc5u;
			for (const char ch : value) {
				hash ^= static_cast<uint8_t>(ch);
				hash *= 0x01000193u;
			}
			return "fnv1a:" + bytesToHex(hash);
		}

		bool writeFileAtomically(const fs::path& target, const uint8_t* bytes, size_t size, std::string& error)
		{
			std::error_code ec;
			fs::create_directories(target.parent_path(), ec);
			if (ec) {
				error = ec.message();
				return false;
			}

			const fs::path temporary = target.parent_path() / (target.filename().string() + ".tmp");
			{
				std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
				if (!file) {
					error = "Could not open temporary file for writing";
					return false;
				}
				file.write(reinterpret_cast<const char*>(bytes), static_cast<std::streamsize>(size));
				file.close();
				if (!file) {
					error = "Could not write temporary file content";
					fs::remove(temporary, ec);
					return false;
				}
			}

			fs::rename(temporary, target, ec);
			if (!ec)
				return true;

			fs::remove(target, ec);
			ec.clear();
			fs::rename(temporary, target, ec);
			if (!ec)
				return true;

			error = ec.message();
			fs::remove(temporary, ec);
			return false;
		}

		bool writeTextFileAtomically(const fs::path& target, std::string_view content, std::string& error)
		{
			return writeFileAtomically(
				target,
				reinterpret_cast<const uint8_t*>(content.data()),
				content.size(),
				error
			);
		}

		std::string buildManifest()
		{
			const fs::path dataRoot = fs::path(DEMO->m_dataFolder).lexically_normal();
			std::vector<std::string> entries;
			std::vector<std::string> errors;

			for (const std::string rootName : { "pool", "resources" }) {
				const fs::path rootPath = dataRoot / rootName;
				if (!fs::exists(rootPath)) {
					errors.push_back(std::format(
						"{{\"path\":\"{}\",\"message\":\"Missing {} folder\"}}",
						rootName,
						rootName
					));
					continue;
				}

				entries.push_back(std::format("{{\"path\":\"{}\",\"kind\":\"directory\"}}", rootName));
				for (const auto& entry : fs::recursive_directory_iterator(rootPath, fs::directory_options::skip_permission_denied)) {
					const fs::path relativePath = fs::relative(entry.path(), dataRoot);
					const std::string relative = relativePath.generic_string();
					if (entry.is_directory()) {
						entries.push_back(std::format(
							"{{\"path\":\"{}\",\"kind\":\"directory\"}}",
							escapeJsonValue(relative)
						));
						continue;
					}

					if (!entry.is_regular_file())
						continue;

					const auto fileSize = entry.file_size();
					const std::string hash = hashFile(entry.path());
					entries.push_back(std::format(
						"{{\"path\":\"{}\",\"kind\":\"file\",\"size\":{},\"hash\":\"{}\"}}",
						escapeJsonValue(relative),
						fileSize,
						escapeJsonValue(hash)
					));
				}
			}

			std::string json = "{\"root\":\"phoenix-engine\",\"generatedAt\":\"\",\"entries\":[";
			for (size_t i = 0; i < entries.size(); ++i) {
				if (i > 0)
					json += ',';
				json += entries[i];
			}
			json += "],\"errors\":[";
			for (size_t i = 0; i < errors.size(); ++i) {
				if (i > 0)
					json += ',';
				json += errors[i];
			}
			json += "]}";
			return json;
		}

		std::vector<uint8_t> decodeBase64(std::string_view input)
		{
			static constexpr unsigned char kInvalid = 255;
			std::array<unsigned char, 256> table{};
			table.fill(kInvalid);
			const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			for (size_t i = 0; i < chars.size(); ++i)
				table[static_cast<unsigned char>(chars[i])] = static_cast<unsigned char>(i);

			std::vector<uint8_t> output;
			uint32_t buffer = 0;
			int bits = 0;
			for (const char ch : input) {
				if (ch == '=')
					break;
				const unsigned char value = table[static_cast<unsigned char>(ch)];
				if (value == kInvalid)
					continue;
				buffer = (buffer << 6) | value;
				bits += 6;
				if (bits >= 8) {
					bits -= 8;
					output.push_back(static_cast<uint8_t>((buffer >> bits) & 0xff));
				}
			}
			return output;
		}

		bool extractBoolean(std::string_view message, std::string_view key, bool& value)
		{
			const std::string quotedKey = std::format("\"{}\"", key);
			size_t pos = message.find(quotedKey);
			if (pos == std::string_view::npos)
				return false;

			pos = message.find(':', pos + quotedKey.size());
			if (pos == std::string_view::npos)
				return false;

			const size_t start = message.find_first_not_of(" \t\r\n", pos + 1);
			if (start == std::string_view::npos)
				return false;

			if (message.substr(start, 4) == "true") {
				value = true;
				return true;
			}
			if (message.substr(start, 5) == "false") {
				value = false;
				return true;
			}
			return false;
		}

		bool isSafeSectionId(std::string_view id)
		{
			if (id.empty())
				return false;

			return std::all_of(id.begin(), id.end(), [](unsigned char ch) {
				return std::isalnum(ch) || ch == '_' || ch == '-';
			});
		}

		std::string decodeBase64Text(std::string_view value)
		{
			const std::vector<uint8_t> bytes = decodeBase64(value);
			return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		}

		std::vector<std::string_view> extractObjectArray(std::string_view message, std::string_view key)
		{
			std::vector<std::string_view> objects;
			const std::string quotedKey = std::format("\"{}\"", key);
			size_t pos = message.find(quotedKey);
			if (pos == std::string_view::npos)
				return objects;

			pos = message.find('[', pos + quotedKey.size());
			if (pos == std::string_view::npos)
				return objects;

			bool inString = false;
			bool escaped = false;
			int depth = 0;
			size_t objectStart = std::string_view::npos;
			for (size_t i = pos + 1; i < message.size(); ++i) {
				const char ch = message[i];
				if (inString) {
					if (escaped) {
						escaped = false;
						continue;
					}
					if (ch == '\\') {
						escaped = true;
						continue;
					}
					if (ch == '"')
						inString = false;
					continue;
				}

				if (ch == '"') {
					inString = true;
					continue;
				}
				if (ch == '{') {
					if (depth == 0)
						objectStart = i;
					++depth;
					continue;
				}
				if (ch == '}') {
					--depth;
					if (depth == 0 && objectStart != std::string_view::npos) {
						objects.push_back(message.substr(objectStart, i - objectStart + 1));
						objectStart = std::string_view::npos;
					}
					continue;
				}
				if (ch == ']' && depth == 0)
					break;
			}

			return objects;
		}

		std::vector<std::string> extractStringArray(std::string_view message, std::string_view key)
		{
			std::vector<std::string> values;
			const std::string quotedKey = std::format("\"{}\"", key);
			size_t pos = message.find(quotedKey);
			if (pos == std::string_view::npos)
				return values;

			pos = message.find('[', pos + quotedKey.size());
			if (pos == std::string_view::npos)
				return values;

			const size_t end = message.find(']', pos + 1);
			if (end == std::string_view::npos)
				return values;

			size_t cursor = pos + 1;
			while (cursor < end) {
				cursor = message.find('"', cursor);
				if (cursor == std::string_view::npos || cursor >= end)
					break;
				++cursor;
				std::string value;
				bool escaped = false;
				for (; cursor < end; ++cursor) {
					const char ch = message[cursor];
					if (escaped) {
						value.push_back(ch);
						escaped = false;
						continue;
					}
					if (ch == '\\') {
						escaped = true;
						continue;
					}
					if (ch == '"')
						break;
					value.push_back(ch);
				}
				values.push_back(std::move(value));
				++cursor;
			}

			return values;
		}

		std::string formatSectionNumber(float value)
		{
			std::ostringstream stream;
			stream << value;
			return stream.str();
		}

		std::string buildSectionContent(const IncomingSection& section)
		{
			std::string content = std::format(
				":::{}\r\nid {}\r\nstart {}\r\nend {}\r\nenabled {}\r\nlayer {}\r\nblend {} {}\r\nblendequation {}\r\n\r\n{}",
				section.type,
				section.id,
				formatSectionNumber(section.startTime),
				formatSectionNumber(section.endTime),
				section.enabled ? 1 : 0,
				section.layer,
				section.srcBlending,
				section.dstBlending,
				section.blendingEQ,
				section.script
			);
			if (!content.ends_with('\n'))
				content += "\r\n";
			return content;
		}

		bool parseIncomingSections(std::string_view body, std::vector<IncomingSection>& sections, std::string& error)
		{
			const auto sectionObjects = extractObjectArray(body, "sections");
			sections.clear();
			sections.reserve(sectionObjects.size());

			for (const auto sectionBody : sectionObjects) {
				IncomingSection section;
				std::string scriptBase64;
				if (!EditorApiServer::extractString(sectionBody, "id", section.id) ||
					!EditorApiServer::extractString(sectionBody, "type", section.type) ||
					!EditorApiServer::extractNumber(sectionBody, "startTime", section.startTime) ||
					!EditorApiServer::extractNumber(sectionBody, "endTime", section.endTime) ||
					!EditorApiServer::extractInteger(sectionBody, "layer", section.layer) ||
					!EditorApiServer::extractString(sectionBody, "srcBlending", section.srcBlending) ||
					!EditorApiServer::extractString(sectionBody, "dstBlending", section.dstBlending) ||
					!EditorApiServer::extractString(sectionBody, "blendingEQ", section.blendingEQ) ||
					!EditorApiServer::extractString(sectionBody, "scriptBase64", scriptBase64) ||
					!extractBoolean(sectionBody, "enabled", section.enabled)) {
					error = "Each section requires id, type, timing, layer, blending, enabled, and scriptBase64";
					return false;
				}

				if (!isSafeSectionId(section.id)) {
					error = std::format("Invalid section id: {}", section.id);
					return false;
				}
				if (section.type.empty() || getSectionType(section.type) == SectionType::NOT_FOUND) {
					error = std::format("Unknown section type: {}", section.type);
					return false;
				}
				if (section.endTime < section.startTime) {
					error = std::format("Invalid time range for section {}", section.id);
					return false;
				}

				section.script = decodeBase64Text(scriptBase64);
				section.content = buildSectionContent(section);
				sections.push_back(std::move(section));
			}

			return true;
		}

		fs::path sectionFilePath(std::string_view id)
		{
			return fs::path(DEMO->m_dataFolder) / (std::string(id) + ".spo");
		}

		void deleteSectionSpoFiles(const std::set<std::string>& keepIds)
		{
			if (!fs::exists(DEMO->m_dataFolder))
				return;

			std::error_code ec;
			for (const auto& entry : fs::directory_iterator(DEMO->m_dataFolder, fs::directory_options::skip_permission_denied, ec)) {
				if (ec)
					break;
				if (!entry.is_regular_file() || entry.path().extension() != ".spo")
					continue;
				const std::string id = entry.path().stem().string();
				if (!isSafeSectionId(id) || keepIds.contains(id))
					continue;
				fs::remove(entry.path(), ec);
				if (ec)
					Logger::error("Editor API: could not delete stale section file {}: {}", entry.path().string(), ec.message());
			}
		}

		std::string extractSpoHeaderValue(std::string_view content, std::string_view key)
		{
			std::stringstream stream{ std::string(content) };
			std::string line;
			const std::string prefix = std::string(key) + " ";
			while (std::getline(stream, line)) {
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				if (line.empty())
					break;
				if (line.starts_with(prefix))
					return line.substr(prefix.size());
			}
			return {};
		}

		std::string buildSectionEntryJson(std::string_view id, std::string_view type, float startTime, float endTime, bool enabled, int32_t layer, std::string_view content)
		{
			std::string srcBlending;
			std::string dstBlending;
			const std::string blend = extractSpoHeaderValue(content, "blend");
			if (!blend.empty()) {
				std::stringstream stream(blend);
				stream >> srcBlending >> dstBlending;
			}
			const std::string blendingEQ = extractSpoHeaderValue(content, "blendequation");

			return std::format(
				"{{\"id\":\"{}\",\"type\":\"{}\",\"startTime\":{},\"endTime\":{},\"enabled\":{},\"layer\":{},\"srcBlending\":\"{}\",\"dstBlending\":\"{}\",\"blendingEQ\":\"{}\",\"contentHash\":\"{}\",\"size\":{}}}",
				escapeJsonValue(id),
				escapeJsonValue(type),
				formatSectionNumber(startTime),
				formatSectionNumber(endTime),
				enabled ? "true" : "false",
				layer,
				escapeJsonValue(srcBlending),
				escapeJsonValue(dstBlending),
				escapeJsonValue(blendingEQ),
				escapeJsonValue(hashString(content)),
				content.size()
			);
		}

		std::string buildSectionsManifest()
		{
			std::vector<std::string> entries;
			const auto& sections = DEMO->m_sectionManager.sections();
			entries.reserve(sections.size());

			for (const auto* section : sections) {
				if (!section)
					continue;
				std::string content;
				const fs::path filePath = sectionFilePath(section->identifier);
				if (fs::exists(filePath)) {
					std::ifstream file(filePath, std::ios::binary);
					content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
				}
				entries.push_back(buildSectionEntryJson(
					section->identifier,
					section->type_str,
					section->startTime,
					section->endTime,
					section->enabled,
					section->layer,
					content
				));
			}

			std::string json = "{\"root\":\"phoenix-engine\",\"entries\":[";
			for (size_t i = 0; i < entries.size(); ++i) {
				if (i > 0)
					json += ',';
				json += entries[i];
			}
			json += "]}";
			return json;
		}

		std::string buildAssetChanged(std::string_view operation, std::string_view path, std::string_view requestId, std::string_view kind, uintmax_t size = 0, std::string_view hash = {})
		{
			std::string entry = std::format(
				"{{\"path\":\"{}\",\"kind\":\"{}\"",
				escapeJsonValue(path),
				escapeJsonValue(kind)
			);
			if (kind == "file") {
				entry += std::format(",\"size\":{}", size);
				if (!hash.empty())
					entry += std::format(",\"hash\":\"{}\"", escapeJsonValue(hash));
			}
			entry += "}";
			return std::format(
				"{{\"type\":\"asset.changed\",\"requestId\":\"{}\",\"operation\":\"{}\",\"path\":\"{}\",\"entry\":{}}}",
				escapeJsonValue(requestId),
				escapeJsonValue(operation),
				escapeJsonValue(path),
				entry
			);
		}

		std::string buildSectionsChanged(std::string_view requestId, size_t count)
		{
			return std::format(
				"{{\"type\":\"section.changed\",\"requestId\":\"{}\",\"operation\":\"replace-all\",\"count\":{}}}",
				escapeJsonValue(requestId),
				count
			);
		}

		std::string buildSectionUpdated(std::string_view requestId, std::string_view id)
		{
			return std::format(
				"{{\"type\":\"section.changed\",\"requestId\":\"{}\",\"operation\":\"update-one\",\"id\":\"{}\",\"count\":1}}",
				escapeJsonValue(requestId),
				escapeJsonValue(id)
			);
		}

		std::string buildSectionsDeleted(std::string_view requestId, size_t count)
		{
			return std::format(
				"{{\"type\":\"section.changed\",\"requestId\":\"{}\",\"operation\":\"delete-many\",\"count\":{}}}",
				escapeJsonValue(requestId),
				count
			);
		}

		std::string buildFailedSectionsJson(const std::vector<std::string>& failedIds)
		{
			std::string json = "[";
			for (size_t i = 0; i < failedIds.size(); ++i) {
				if (i > 0)
					json += ',';
				json += std::format(
					"{{\"id\":\"{}\",\"message\":\"Could not load section {}\"}}",
					escapeJsonValue(failedIds[i]),
					escapeJsonValue(failedIds[i])
				);
			}
			json += "]";
			return json;
		}

		SectionReplaceResult replaceSectionsOnMainThread(const std::vector<IncomingSection>& sections, std::string_view requestId)
		{
			const size_t totalSteps = sections.size() * 2;
			updateSectionSyncStatus(requestId, "replace-all", "validating", 0, totalSteps, 0, 0, false, "Validating sections...");

			std::set<std::string> incomingIds;
			for (const auto& section : sections) {
				if (!incomingIds.insert(section.id).second) {
					updateSectionSyncStatus(requestId, "replace-all", "error", 0, totalSteps, 0, 1, true, std::format("Duplicate section id: {}", section.id));
					return {
						.status = "400 Bad Request",
						.body = buildAssetError("duplicate-section", std::format("Duplicate section id: {}", section.id), requestId)
					};
				}
			}

			try {
				size_t written = 0;
				for (const auto& section : sections) {
					updateSectionSyncStatus(requestId, "replace-all", "writing", written, totalSteps, 0, 0, false, std::format("Writing section {}...", section.id));
					std::string writeError;
					if (!writeTextFileAtomically(sectionFilePath(section.id), section.content, writeError)) {
						updateSectionSyncStatus(requestId, "replace-all", "error", written, totalSteps, 0, 1, true, writeError);
						return {
							.status = "500 Internal Server Error",
							.body = buildAssetError("section-write-failed", writeError, requestId)
						};
					}
					++written;
					updateSectionSyncStatus(requestId, "replace-all", "writing", written, totalSteps, 0, 0, false, std::format("Wrote section {}", section.id));
				}

				updateSectionSyncStatus(requestId, "replace-all", "clearing", written, totalSteps, 0, 0, false, "Replacing Phoenix sections...");
				deleteSectionSpoFiles(incomingIds);
				DEMO->m_sectionManager.clear();

				size_t loaded = 0;
				std::vector<std::string> failedIds;
				size_t processed = 0;
				for (const auto& section : sections) {
					updateSectionSyncStatus(requestId, "replace-all", "loading", written + processed, totalSteps, loaded, failedIds.size(), false, std::format("Loading section {}...", section.id));
					if (DEMO->loadScriptFromNetwork(section.content))
						++loaded;
					else
						failedIds.push_back(section.id);
					++processed;
					updateSectionSyncStatus(requestId, "replace-all", "loading", written + processed, totalSteps, loaded, failedIds.size(), false, std::format("Loaded {}/{} sections", loaded, sections.size()));
				}

				updateSectionSyncStatus(requestId, "replace-all", failedIds.empty() ? "complete" : "error", totalSteps, totalSteps, loaded, failedIds.size(), true, failedIds.empty() ? "Phoenix sections sync complete." : "Phoenix sections sync finished with issues.");
				return {
					.status = "200 OK",
					.body = std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"replace-all\",\"received\":{},\"loaded\":{},\"failed\":{},\"writtenFiles\":{},\"deletedFiles\":[],\"failedSections\":{},\"manifest\":{}}}",
						escapeJsonValue(requestId),
						sections.size(),
						loaded,
						failedIds.size(),
						sections.size(),
						buildFailedSectionsJson(failedIds),
						buildSectionsManifest()
					),
					.eventPayload = buildSectionsChanged(requestId, sections.size())
				};
			}
			catch (const std::exception& err) {
				updateSectionSyncStatus(requestId, "replace-all", "error", 0, totalSteps, 0, 1, true, err.what());
				return {
					.status = "500 Internal Server Error",
					.body = buildAssetError("section-sync-failed", err.what(), requestId)
				};
			}
		}

		SectionReplaceResult updateSectionOnMainThread(const std::vector<IncomingSection>& sections, std::string_view requestId)
		{
			if (sections.size() != 1) {
				return {
					.status = "400 Bad Request",
					.body = buildAssetError("invalid-section-count", "Single section update requires exactly one section", requestId)
				};
			}

			const auto& section = sections.front();
			try {
				DEMO->m_sectionManager.deleteSections({ section.id });
				const bool loaded = DEMO->loadScriptFromNetwork(section.content);

				std::string writeError;
				if (!writeTextFileAtomically(sectionFilePath(section.id), section.content, writeError)) {
					return {
						.status = "500 Internal Server Error",
						.body = buildAssetError("section-write-failed", writeError, requestId)
					};
				}

				std::vector<std::string> failedIds;
				if (!loaded)
					failedIds.push_back(section.id);

				return {
					.status = "200 OK",
					.body = std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"update-one\",\"received\":1,\"loaded\":{},\"failed\":{},\"writtenFiles\":1,\"deletedFiles\":[],\"failedSections\":{},\"manifest\":{}}}",
						escapeJsonValue(requestId),
						loaded ? 1 : 0,
						loaded ? 0 : 1,
						buildFailedSectionsJson(failedIds),
						buildSectionsManifest()
					),
					.eventPayload = buildSectionUpdated(requestId, section.id)
				};
			}
			catch (const std::exception& err) {
				return {
					.status = "500 Internal Server Error",
					.body = buildAssetError("section-update-failed", err.what(), requestId)
				};
			}
		}

		SectionReplaceResult deleteSectionsOnMainThread(const std::vector<std::string>& sectionIds, std::string_view requestId)
		{
			std::set<std::string> ids;
			for (const auto& id : sectionIds) {
				if (!isSafeSectionId(id)) {
					return {
						.status = "400 Bad Request",
						.body = buildAssetError("invalid-section-id", std::format("Invalid section id: {}", id), requestId)
					};
				}
				ids.insert(id);
			}

			try {
				std::vector<std::string> idsToDelete(ids.begin(), ids.end());
				DEMO->m_sectionManager.deleteSections(idsToDelete);

				std::vector<std::string> deletedFiles;
				std::error_code ec;
				for (const auto& id : ids) {
					const fs::path filePath = sectionFilePath(id);
					if (!fs::exists(filePath))
						continue;
					fs::remove(filePath, ec);
					if (ec) {
						return {
							.status = "500 Internal Server Error",
							.body = buildAssetError("section-delete-failed", ec.message(), requestId)
						};
					}
					deletedFiles.push_back(filePath.filename().generic_string());
				}

				std::string deletedJson = "[";
				for (size_t i = 0; i < deletedFiles.size(); ++i) {
					if (i > 0)
						deletedJson += ',';
					deletedJson += std::format("\"{}\"", escapeJsonValue(deletedFiles[i]));
				}
				deletedJson += "]";

				return {
					.status = "200 OK",
					.body = std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"delete-many\",\"received\":{},\"loaded\":0,\"failed\":0,\"writtenFiles\":0,\"deletedFiles\":{},\"failedSections\":[],\"manifest\":{}}}",
						escapeJsonValue(requestId),
						ids.size(),
						deletedJson,
						buildSectionsManifest()
					),
					.eventPayload = buildSectionsDeleted(requestId, ids.size())
				};
			}
			catch (const std::exception& err) {
				return {
					.status = "500 Internal Server Error",
					.body = buildAssetError("section-delete-failed", err.what(), requestId)
				};
			}
		}

		ImGuiKey glfwKeyToImGuiKey(int32_t key)
		{
			if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
				return static_cast<ImGuiKey>(ImGuiKey_A + (key - GLFW_KEY_A));
			if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
				return static_cast<ImGuiKey>(ImGuiKey_0 + (key - GLFW_KEY_0));
			if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12)
				return static_cast<ImGuiKey>(ImGuiKey_F1 + (key - GLFW_KEY_F1));
			if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9)
				return static_cast<ImGuiKey>(ImGuiKey_Keypad0 + (key - GLFW_KEY_KP_0));

			switch (key) {
			case GLFW_KEY_TAB: return ImGuiKey_Tab;
			case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
			case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
			case GLFW_KEY_UP: return ImGuiKey_UpArrow;
			case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
			case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
			case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
			case GLFW_KEY_HOME: return ImGuiKey_Home;
			case GLFW_KEY_END: return ImGuiKey_End;
			case GLFW_KEY_INSERT: return ImGuiKey_Insert;
			case GLFW_KEY_DELETE: return ImGuiKey_Delete;
			case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
			case GLFW_KEY_SPACE: return ImGuiKey_Space;
			case GLFW_KEY_ENTER: return ImGuiKey_Enter;
			case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
			case GLFW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
			case GLFW_KEY_COMMA: return ImGuiKey_Comma;
			case GLFW_KEY_MINUS: return ImGuiKey_Minus;
			case GLFW_KEY_PERIOD: return ImGuiKey_Period;
			case GLFW_KEY_SLASH: return ImGuiKey_Slash;
			case GLFW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
			case GLFW_KEY_EQUAL: return ImGuiKey_Equal;
			case GLFW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
			case GLFW_KEY_BACKSLASH: return ImGuiKey_Backslash;
			case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
			case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
			case GLFW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
			case GLFW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
			case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
			case GLFW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
			case GLFW_KEY_PAUSE: return ImGuiKey_Pause;
			case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
			case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
			case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
			case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
			case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
			case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
			case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
			case GLFW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
			case GLFW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
			case GLFW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
			case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
			case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
			case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
			case GLFW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
			case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
			case GLFW_KEY_MENU: return ImGuiKey_Menu;
			default: return ImGuiKey_None;
			}
		}

		template <typename T>
		void addRemoteKeyEventToImGui(const T& event, bool down)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddKeyEvent(ImGuiMod_Shift, (event.mods & GLFW_MOD_SHIFT) != 0);
			io.AddKeyEvent(ImGuiMod_Ctrl, (event.mods & GLFW_MOD_CONTROL) != 0);
			io.AddKeyEvent(ImGuiMod_Alt, (event.mods & GLFW_MOD_ALT) != 0);
			io.AddKeyEvent(ImGuiMod_Super, (event.mods & GLFW_MOD_SUPER) != 0);
			const ImGuiKey imguiKey = glfwKeyToImGuiKey(event.key);
			if (imguiKey != ImGuiKey_None)
				io.AddKeyEvent(imguiKey, down);
		}
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
		m_remoteMouseActive(false),
		m_remoteMouseX(0.0f),
		m_remoteMouseY(0.0f),
		m_remoteMouseWheelX(0.0f),
		m_remoteMouseWheelY(0.0f),
		m_remoteMouseButtons(),
		m_remoteMouseReleaseFrames(),
		m_remoteKeys(),
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

		processSectionReplaceRequests();
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
		clearRemoteKeys();

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
				->writeHeader("Access-Control-Allow-Private-Network", "true")
				->writeHeader("Content-Type", "application/json")
				->end(std::format(
					"{{\"status\":\"ok\",\"service\":\"phoenix-editor-api\",\"port\":{}}}",
					m_port
				));
		});

		app.get("/api/assets/manifest", [](auto* response, auto*) {
			sendJson(response, "200 OK", buildManifest());
		});

		app.get("/api/assets", [](auto* response, auto*) {
			sendJson(response, "200 OK", buildManifest());
		});

		app.put("/api/assets/file", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				std::string path;
				std::string encoding;
				std::string content;
				extractString(*body, "requestId", requestId);
				if (!extractString(*body, "path", path) || !extractString(*body, "encoding", encoding) || !extractString(*body, "content", content)) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-request", "write file requires path, encoding, and content", requestId));
					return;
				}
				if (encoding != "base64") {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-encoding", "Only base64 content is supported", requestId));
					return;
				}

				const auto assetPath = resolveAssetPath(path);
				if (!assetPath) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-path", "Asset path must be under pool or resources", requestId));
					return;
				}

				try {
					const std::vector<uint8_t> bytes = decodeBase64(content);
					std::string writeError;
					if (!writeFileAtomically(assetPath->full, bytes.data(), bytes.size(), writeError)) {
						sendJson(response, "500 Internal Server Error", buildAssetError("write-failed", writeError, requestId));
						return;
					}
					const std::string fileHash = hashFile(assetPath->full);
					sendJson(response, "200 OK", std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"write-file\",\"entry\":{{\"path\":\"{}\",\"kind\":\"file\",\"size\":{},\"hash\":\"{}\"}}}}",
						escapeJsonValue(requestId),
						escapeJsonValue(assetPath->relative),
						bytes.size(),
						escapeJsonValue(fileHash)
					));
					publishEvent(buildAssetChanged("write-file", assetPath->relative, requestId, "file", bytes.size(), fileHash));
				}
				catch (const std::exception& err) {
					sendJson(response, "500 Internal Server Error", buildAssetError("write-failed", err.what(), requestId));
				}
			});
			response->onAborted([body]() {});
		});

		app.del("/api/assets/file", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				std::string path;
				extractString(*body, "requestId", requestId);
				if (!extractString(*body, "path", path)) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-request", "delete file requires path", requestId));
					return;
				}

				const auto assetPath = resolveAssetPath(path);
				if (!assetPath) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-path", "Asset path must be under pool or resources", requestId));
					return;
				}

				try {
					std::error_code ec;
					fs::remove(assetPath->full, ec);
					if (ec) {
						sendJson(response, "500 Internal Server Error", buildAssetError("delete-failed", ec.message(), requestId));
						return;
					}
					sendJson(response, "200 OK", std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"delete-file\",\"entry\":{{\"path\":\"{}\",\"kind\":\"file\"}}}}",
						escapeJsonValue(requestId),
						escapeJsonValue(assetPath->relative)
					));
					publishEvent(buildAssetChanged("delete-file", assetPath->relative, requestId, "file"));
				}
				catch (const std::exception& err) {
					sendJson(response, "500 Internal Server Error", buildAssetError("delete-failed", err.what(), requestId));
				}
			});
			response->onAborted([body]() {});
		});

		app.post("/api/assets/directory", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				std::string path;
				extractString(*body, "requestId", requestId);
				if (!extractString(*body, "path", path)) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-request", "create directory requires path", requestId));
					return;
				}

				const auto assetPath = resolveAssetPath(path);
				if (!assetPath) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-path", "Asset path must be under pool or resources", requestId));
					return;
				}

				try {
					fs::create_directories(assetPath->full);
					sendJson(response, "200 OK", std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"create-directory\",\"entry\":{{\"path\":\"{}\",\"kind\":\"directory\"}}}}",
						escapeJsonValue(requestId),
						escapeJsonValue(assetPath->relative)
					));
					publishEvent(buildAssetChanged("create-directory", assetPath->relative, requestId, "directory"));
				}
				catch (const std::exception& err) {
					sendJson(response, "500 Internal Server Error", buildAssetError("create-directory-failed", err.what(), requestId));
				}
			});
			response->onAborted([body]() {});
		});

		app.del("/api/assets/directory", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				std::string path;
				bool recursive = false;
				extractString(*body, "requestId", requestId);
				extractBoolean(*body, "recursive", recursive);
				if (!extractString(*body, "path", path)) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-request", "delete directory requires path", requestId));
					return;
				}

				const auto assetPath = resolveAssetPath(path);
				if (!assetPath) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-path", "Asset path must be under pool or resources", requestId));
					return;
				}

				try {
					std::error_code ec;
					if (recursive)
						fs::remove_all(assetPath->full, ec);
					else
						fs::remove(assetPath->full, ec);
					if (ec) {
						sendJson(response, "500 Internal Server Error", buildAssetError("delete-directory-failed", ec.message(), requestId));
						return;
					}
					sendJson(response, "200 OK", std::format(
						"{{\"requestId\":\"{}\",\"ok\":true,\"operation\":\"delete-directory\",\"entry\":{{\"path\":\"{}\",\"kind\":\"directory\"}}}}",
						escapeJsonValue(requestId),
						escapeJsonValue(assetPath->relative)
					));
					publishEvent(buildAssetChanged("delete-directory", assetPath->relative, requestId, "directory"));
				}
				catch (const std::exception& err) {
					sendJson(response, "500 Internal Server Error", buildAssetError("delete-directory-failed", err.what(), requestId));
				}
			});
			response->onAborted([body]() {});
		});

		app.get("/api/sections/manifest", [](auto* response, auto*) {
			sendJson(response, "200 OK", buildSectionsManifest());
		});

		app.get("/api/sections/status/:requestId", [](auto* response, auto* request) {
			const std::string_view requestId = request->getParameter(0);
			const std::string body = buildSectionSyncStatusJson(requestId);
			const bool ok = body.find("\"ok\":true") != std::string::npos;
			sendJson(response, ok ? "200 OK" : "404 Not Found", body);
		});

		app.put("/api/sections", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				extractString(*body, "requestId", requestId);

				std::vector<IncomingSection> sections;
				std::string parseError;
				if (!parseIncomingSections(*body, sections, parseError)) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-sections", parseError, requestId));
					return;
				}

				const auto request = std::make_shared<SectionReplaceRequest>();
				request->requestId = requestId;
				request->sections = std::move(sections);
				request->operation = SectionRequestOperation::ReplaceAll;
				{
					std::lock_guard lock(kSectionReplaceQueueMutex);
					kSectionReplaceQueue.push(request);
				}

				std::unique_lock lock(request->mutex);
				if (!request->condition.wait_for(lock, std::chrono::seconds(30), [&request]() { return request->done; })) {
					sendJson(response, "504 Gateway Timeout", buildAssetError("section-sync-timeout", "Timed out waiting for Phoenix to apply sections", requestId));
					return;
				}
				sendJson(response, request->result.status, request->result.body);
			});
			response->onAborted([body]() {});
		});

		app.put("/api/sections/section", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				extractString(*body, "requestId", requestId);

				std::vector<IncomingSection> sections;
				std::string parseError;
				if (!parseIncomingSections(*body, sections, parseError)) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-sections", parseError, requestId));
					return;
				}
				if (sections.size() != 1) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-section-count", "Single section update requires exactly one section", requestId));
					return;
				}

				const auto request = std::make_shared<SectionReplaceRequest>();
				request->requestId = requestId;
				request->sections = std::move(sections);
				request->operation = SectionRequestOperation::UpdateOne;
				{
					std::lock_guard lock(kSectionReplaceQueueMutex);
					kSectionReplaceQueue.push(request);
				}

				std::unique_lock lock(request->mutex);
				if (!request->condition.wait_for(lock, std::chrono::seconds(30), [&request]() { return request->done; })) {
					sendJson(response, "504 Gateway Timeout", buildAssetError("section-sync-timeout", "Timed out waiting for Phoenix to apply section", requestId));
					return;
				}
				sendJson(response, request->result.status, request->result.body);
			});
			response->onAborted([body]() {});
		});

		app.del("/api/sections", [this](auto* response, auto*) {
			auto body = std::make_shared<std::string>();
			response->onData([this, response, body](std::string_view chunk, bool last) {
				body->append(chunk);
				if (!last)
					return;

				std::string requestId;
				extractString(*body, "requestId", requestId);
				auto ids = extractStringArray(*body, "ids");
				if (ids.empty()) {
					sendJson(response, "400 Bad Request", buildAssetError("invalid-section-ids", "delete sections requires ids", requestId));
					return;
				}

				const auto request = std::make_shared<SectionReplaceRequest>();
				request->requestId = requestId;
				request->sectionIds = std::move(ids);
				request->operation = SectionRequestOperation::DeleteMany;
				{
					std::lock_guard lock(kSectionReplaceQueueMutex);
					kSectionReplaceQueue.push(request);
				}

				std::unique_lock lock(request->mutex);
				if (!request->condition.wait_for(lock, std::chrono::seconds(30), [&request]() { return request->done; })) {
					sendJson(response, "504 Gateway Timeout", buildAssetError("section-delete-timeout", "Timed out waiting for Phoenix to delete sections", requestId));
					return;
				}
				sendJson(response, request->result.status, request->result.body);
			});
			response->onAborted([body]() {});
		});

		app.options("/*", [](auto* response, auto*) {
			writeCors(response);
			response->end();
		});

		app.ws<WebSocketData>("/ws", {
			.open = [](auto* ws) {
				auto* data = ws->getUserData();
				data->clientId = kNextWebSocketClientId++;
				data->webRtcPreviewRequested = false;
				ws->subscribe(kRuntimeTopic);
				Logger::info(LogLevel::high, "Editor API: Cacablu WebSocket client {} connected", data->clientId);
			},
			.message = [this](auto* ws, std::string_view message, uWS::OpCode opCode) {
				if (opCode != uWS::OpCode::TEXT) {
					enqueueError("invalid-opcode", "Only text WebSocket messages are supported");
					return;
				}
				std::string type;
				if (extractMessageType(message, type) && type.starts_with("webrtc.")) {
					auto* data = ws->getUserData();
					if (type == "webrtc.enable")
						data->webRtcPreviewRequested = true;
					else if (type == "webrtc.disable")
						data->webRtcPreviewRequested = false;

					std::vector<std::string> deferredSignals;
					const std::string response = handleWebRtcMessage(message, data->clientId, [this, &deferredSignals](std::string_view signalType, std::string_view payload, std::string_view sdpMid) {
						if (signalType == "webrtc.ice-candidate")
							deferredSignals.emplace_back(buildWebRtcCandidateMessage(payload, sdpMid));
					});
					if (!response.empty())
						ws->send(response, uWS::OpCode::TEXT);
					for (const std::string& signal : deferredSignals)
						ws->send(signal, uWS::OpCode::TEXT);
					return;
				}
				enqueueMessage(message);
			},
			.close = [](auto* ws, int, std::string_view) {
				auto* data = ws->getUserData();
				Logger::info(LogLevel::high, "Editor API: Cacablu WebSocket client {} disconnected", data->clientId);
				if (data->webRtcPreviewRequested) {
					Command command{};
					command.type = CommandType::DisableWebRtc;
					command.clientId = data->clientId;
					{
						std::lock_guard lock(EditorApiServer::getInstance().m_commandMutex);
						EditorApiServer::getInstance().m_commands.emplace(command);
					}
				}
				EditorApiServer::getInstance().clearRemoteKeys();
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
		else if (type == "input.mouse.move" || type == "input.mouse.down" || type == "input.mouse.up" || type == "input.mouse.wheel") {
			float x = 0.0f;
			float y = 0.0f;
			if (!extractNumber(message, "x", x) || !extractNumber(message, "y", y) || !std::isfinite(x) || !std::isfinite(y)) {
				enqueueError("invalid-input", "mouse input requires finite x and y coordinates");
				return;
			}
			if (DEMO->m_framebufferStreamer)
				DEMO->m_framebufferStreamer->mapPreviewPointToSource(x, y);

			command.x = x;
			command.y = y;

			if (type == "input.mouse.move") {
				command.type = CommandType::MouseMove;
			}
			else if (type == "input.mouse.down" || type == "input.mouse.up") {
				int32_t button = 0;
				if (!extractInteger(message, "button", button) || button < 0 || button >= static_cast<int32_t>(m_remoteMouseButtons.size())) {
					enqueueError("invalid-input", "mouse button input requires a valid button");
					return;
				}
				command.type = type == "input.mouse.down" ? CommandType::MouseDown : CommandType::MouseUp;
				command.button = button;
			}
			else {
				float deltaX = 0.0f;
				float deltaY = 0.0f;
				if (!extractNumber(message, "deltaX", deltaX) || !extractNumber(message, "deltaY", deltaY) || !std::isfinite(deltaX) || !std::isfinite(deltaY)) {
					enqueueError("invalid-input", "mouse wheel input requires finite deltaX and deltaY");
					return;
				}
				command.type = CommandType::MouseWheel;
				command.deltaX = deltaX;
				command.deltaY = deltaY;
			}
		}
		else if (type == "input.key.down" || type == "input.key.up") {
			int32_t key = 0;
			if (!extractInteger(message, "key", key) || key < 0 || key > GLFW_KEY_LAST) {
				enqueueError("invalid-input", "key input requires a valid GLFW key code");
				return;
			}

			int32_t scancode = 0;
			int32_t mods = 0;
			extractInteger(message, "scancode", scancode);
			extractInteger(message, "mods", mods);

			command.type = type == "input.key.down" ? CommandType::KeyDown : CommandType::KeyUp;
			command.key = key;
			command.scancode = scancode;
			command.mods = mods;
			command.repeat = message.find("\"repeat\":true") != std::string_view::npos;
		}
		else {
			enqueueError("unsupported-message", std::format("Unsupported message type: {}", type));
			return;
		}

		std::lock_guard lock(m_commandMutex);
		m_commands.emplace(command);
	}

	std::string EditorApiServer::handleWebRtcMessage(std::string_view message, int32_t clientId, const WebRtcSignalSender& signalSender)
	{
		std::string type;
		if (!extractMessageType(message, type))
			return buildErrorMessage("invalid-message", "Missing message type");

		if (type == "webrtc.enable" || type == "webrtc.disable") {
			Command command{};
			command.type = type == "webrtc.enable" ? CommandType::EnableWebRtc : CommandType::DisableWebRtc;
			command.clientId = clientId;
			{
				std::lock_guard lock(m_commandMutex);
				m_commands.emplace(command);
			}
			return buildWebRtcStateMessage(type == "webrtc.enable" ? "enabling" : "disabling");
		}

		if (!DEMO->m_framebufferStreamer || !DEMO->m_framebufferStreamer->isRunning())
			return buildErrorMessage("streaming-disabled", "Phoenix WebRTC preview streaming is not enabled");

		if (type == "webrtc.request") {
			const auto offer = DEMO->m_framebufferStreamer->createOffer([](std::string_view, std::string_view, std::string_view) {});
			if (offer.sdp.empty())
				return buildErrorMessage("offer-failed", "Phoenix could not create a WebRTC offer");
			Logger::info(LogLevel::high, "Editor API: WebRTC offer created for session {}", offer.sessionId);
			return buildWebRtcOfferMessage(offer.sessionId, offer.sdp);
		}

		if (type == "webrtc.answer") {
			std::string sdp;
			int32_t sessionId = 0;
			if (!extractString(message, "sdp", sdp) || sdp.empty())
				return buildErrorMessage("invalid-answer", "webrtc.answer requires SDP");
			if (!extractInteger(message, "sessionId", sessionId) || sessionId <= 0)
				return buildErrorMessage("invalid-session", "webrtc.answer requires a sessionId");
			Logger::info(LogLevel::high, "Editor API: WebRTC answer received for session {}", sessionId);
			DEMO->m_framebufferStreamer->handleAnswer(sessionId, sdp);
			return {};
		}

		if (type == "webrtc.ice-candidate") {
			std::string candidate;
			std::string sdpMid;
			int32_t sessionId = 0;
			int32_t sdpMLineIndex = -1;
			if (!extractString(message, "candidate", candidate) || candidate.empty())
				return {};
			if (!extractInteger(message, "sessionId", sessionId) || sessionId <= 0)
				return buildErrorMessage("invalid-session", "webrtc.ice-candidate requires a sessionId");
			Logger::info(LogLevel::high, "Editor API: WebRTC ICE candidate received for session {}", sessionId);
			extractString(message, "sdpMid", sdpMid);
			extractInteger(message, "sdpMLineIndex", sdpMLineIndex);
			DEMO->m_framebufferStreamer->handleRemoteCandidate(sessionId, candidate, sdpMid, sdpMLineIndex);
			return {};
		}

		return buildErrorMessage("unsupported-message", std::format("Unsupported message type: {}", type));
	}

	void EditorApiServer::enqueueError(std::string_view code, std::string_view message)
	{
		publishEvent(buildErrorMessage(code, message));
	}

	void EditorApiServer::publishEvent(std::string_view payload)
	{
		if (!m_loop || !m_app)
			return;

		const std::string message(payload);
		m_loop->defer([this, message]() {
			if (m_app)
				m_app->publish(kRuntimeTopic, message, uWS::OpCode::TEXT);
		});
	}

	void EditorApiServer::applyRemoteInputToImGui()
	{
		if ((!m_remoteMouseActive && m_remoteInputEvents.empty()) || !ImGui::GetCurrentContext())
			return;

#if IMGUI_VERSION_NUM >= 18700
		GLFWwindow* window = static_cast<GLFWwindow*>(DEMO->GetWindow().GetNativeWindow());
		if (!window)
			return;

		for (const RemoteInputEvent& event : m_remoteInputEvents) {
			if (event.type == CommandType::MouseMove || event.type == CommandType::MouseDown || event.type == CommandType::MouseUp || event.type == CommandType::MouseWheel)
				ImGui_ImplGlfw_CursorPosCallback(window, event.x, event.y);
			if (event.type == CommandType::MouseDown)
				ImGui_ImplGlfw_MouseButtonCallback(window, event.button, GLFW_PRESS, 0);
			else if (event.type == CommandType::MouseUp)
				ImGui_ImplGlfw_MouseButtonCallback(window, event.button, GLFW_RELEASE, 0);
			else if (event.type == CommandType::MouseWheel)
				ImGui_ImplGlfw_ScrollCallback(window, event.deltaX, event.deltaY);
			else if (event.type == CommandType::KeyDown)
				addRemoteKeyEventToImGui(event, true);
			else if (event.type == CommandType::KeyUp)
				addRemoteKeyEventToImGui(event, false);
		}

		m_remoteInputEvents = std::move(m_nextFrameRemoteInputEvents);
		m_nextFrameRemoteInputEvents.clear();
		m_remoteMouseWheelX = 0.0f;
		m_remoteMouseWheelY = 0.0f;
#else
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(m_remoteMouseX, m_remoteMouseY);
		for (size_t i = 0; i < m_remoteMouseButtons.size() && i < IM_ARRAYSIZE(io.MouseDown); ++i)
			io.MouseDown[i] = m_remoteMouseButtons[i];
		io.MouseWheel += m_remoteMouseWheelY;
		for (size_t i = 0; i < m_remoteMouseReleaseFrames.size(); ++i) {
			if (m_remoteMouseReleaseFrames[i] > 0) {
				--m_remoteMouseReleaseFrames[i];
				if (m_remoteMouseReleaseFrames[i] == 0)
					m_remoteMouseButtons[i] = false;
			}
		}
		m_remoteMouseWheelX = 0.0f;
		m_remoteMouseWheelY = 0.0f;
#endif
	}

	bool EditorApiServer::isRemoteKeyPressed(int32_t key) const
	{
		if (key < 0 || key >= static_cast<int32_t>(m_remoteKeys.size()))
			return false;

		std::lock_guard lock(m_remoteKeyMutex);
		return m_remoteKeys[static_cast<size_t>(key)];
	}

	void EditorApiServer::clearRemoteKeys()
	{
		std::lock_guard lock(m_remoteKeyMutex);
		m_remoteKeys.fill(false);
	}

	void EditorApiServer::setRemoteKeyPressed(int32_t key, bool pressed)
	{
		if (key < 0 || key >= static_cast<int32_t>(m_remoteKeys.size()))
			return;

		std::lock_guard lock(m_remoteKeyMutex);
		m_remoteKeys[static_cast<size_t>(key)] = pressed;
	}

	void EditorApiServer::processSectionReplaceRequests()
	{
		while (true) {
			std::shared_ptr<SectionReplaceRequest> request;
			{
				std::lock_guard lock(kSectionReplaceQueueMutex);
				if (kSectionReplaceQueue.empty())
					return;
				request = kSectionReplaceQueue.front();
				kSectionReplaceQueue.pop();
			}

			SectionReplaceResult result;
			switch (request->operation) {
			case SectionRequestOperation::ReplaceAll:
				result = replaceSectionsOnMainThread(request->sections, request->requestId);
				break;
			case SectionRequestOperation::UpdateOne:
				result = updateSectionOnMainThread(request->sections, request->requestId);
				break;
			case SectionRequestOperation::DeleteMany:
				result = deleteSectionsOnMainThread(request->sectionIds, request->requestId);
				break;
			}
			if (!result.eventPayload.empty())
				publishEvent(result.eventPayload);

			{
				std::lock_guard lock(request->mutex);
				request->result = std::move(result);
				request->done = true;
			}
			request->condition.notify_all();
		}
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
			case CommandType::MouseMove: {
				m_remoteMouseActive = true;
				m_remoteMouseX = command.x;
				m_remoteMouseY = command.y;
				m_remoteInputEvents.push_back(command);
				MouseMovedEvent event(command.x, command.y);
				DEMO->OnEvent(event);
				break;
			}
			case CommandType::MouseDown: {
				m_remoteMouseActive = true;
				m_remoteMouseX = command.x;
				m_remoteMouseY = command.y;
				if (command.button >= 0 && command.button < static_cast<int32_t>(m_remoteMouseButtons.size()))
					m_remoteMouseButtons[command.button] = true;
				if (command.button >= 0 && command.button < static_cast<int32_t>(m_remoteMouseReleaseFrames.size()))
					m_remoteMouseReleaseFrames[command.button] = 0;
				m_remoteInputEvents.push_back(command);
				MouseMovedEvent moveEvent(command.x, command.y);
				DEMO->OnEvent(moveEvent);
				MouseButtonPressedEvent buttonEvent(static_cast<MouseCode>(command.button));
				DEMO->OnEvent(buttonEvent);
				break;
			}
			case CommandType::MouseUp: {
				m_remoteMouseActive = true;
				m_remoteMouseX = command.x;
				m_remoteMouseY = command.y;
				if (command.button >= 0 && command.button < static_cast<int32_t>(m_remoteMouseButtons.size()))
					m_remoteMouseReleaseFrames[command.button] = 1;
				m_nextFrameRemoteInputEvents.push_back(command);
				MouseMovedEvent moveEvent(command.x, command.y);
				DEMO->OnEvent(moveEvent);
				MouseButtonReleasedEvent buttonEvent(static_cast<MouseCode>(command.button));
				DEMO->OnEvent(buttonEvent);
				break;
			}
			case CommandType::MouseWheel: {
				m_remoteMouseActive = true;
				m_remoteMouseX = command.x;
				m_remoteMouseY = command.y;
				m_remoteMouseWheelX += command.deltaX;
				m_remoteMouseWheelY += command.deltaY;
				m_remoteInputEvents.push_back(command);
				MouseMovedEvent moveEvent(command.x, command.y);
				DEMO->OnEvent(moveEvent);
				MouseScrolledEvent wheelEvent(command.deltaX, command.deltaY);
				DEMO->OnEvent(wheelEvent);
				break;
			}
			case CommandType::KeyDown: {
				setRemoteKeyPressed(command.key, true);
				m_remoteInputEvents.push_back(command);
				KeyPressedEvent event(static_cast<KeyCode>(command.key), command.repeat ? 1 : 0);
				DEMO->OnEvent(event);
				break;
			}
			case CommandType::KeyUp: {
				setRemoteKeyPressed(command.key, false);
				m_remoteInputEvents.push_back(command);
				KeyReleasedEvent event(static_cast<KeyCode>(command.key));
				DEMO->OnEvent(event);
				break;
			}
			case CommandType::EnableWebRtc:
				m_webRtcPreviewClients.insert(command.clientId);
				if (!DEMO->m_enableStreaming)
					DEMO->setStreamingEnabled(true);
				publishEvent(buildWebRtcStateMessage("enabled"));
				break;
			case CommandType::DisableWebRtc:
				m_webRtcPreviewClients.erase(command.clientId);
				if (m_webRtcPreviewClients.empty() && DEMO->m_enableStreaming) {
					DEMO->setStreamingEnabled(false);
					publishEvent(buildWebRtcStateMessage("disabled"));
				}
				else {
					publishEvent(buildWebRtcStateMessage(DEMO->m_enableStreaming ? "enabled" : "disabled"));
				}
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
			escapeJson(code),
			escapeJson(message)
		);
	}

	std::string EditorApiServer::buildWebRtcAnswerMessage(std::string_view sdp) const
	{
		return std::format(
			"{{\"type\":\"webrtc.answer\",\"sdp\":\"{}\"}}",
			escapeJson(sdp)
		);
	}

	std::string EditorApiServer::buildWebRtcOfferMessage(int32_t sessionId, std::string_view sdp) const
	{
		return std::format(
			"{{\"type\":\"webrtc.offer\",\"sessionId\":{},\"sdp\":\"{}\"}}",
			sessionId,
			escapeJson(sdp)
		);
	}

	std::string EditorApiServer::buildWebRtcCandidateMessage(std::string_view candidate, std::string_view sdpMid) const
	{
		return std::format(
			"{{\"type\":\"webrtc.ice-candidate\",\"candidate\":\"{}\",\"sdpMid\":\"{}\",\"sdpMLineIndex\":0}}",
			escapeJson(candidate),
			escapeJson(sdpMid)
		);
	}

	std::string EditorApiServer::buildWebRtcStateMessage(std::string_view state) const
	{
		return std::format(
			"{{\"type\":\"webrtc.state\",\"state\":\"{}\"}}",
			escapeJson(state)
		);
	}

	bool EditorApiServer::extractMessageType(std::string_view message, std::string& type)
	{
		return extractString(message, "type", type);
	}

	bool EditorApiServer::extractString(std::string_view message, std::string_view key, std::string& value)
	{
		const std::string quotedKey = std::format("\"{}\"", key);
		size_t pos = message.find(quotedKey);
		if (pos == std::string_view::npos)
			return false;

		pos = message.find(':', pos + quotedKey.size());
		if (pos == std::string_view::npos)
			return false;

		pos = message.find('"', pos + 1);
		if (pos == std::string_view::npos)
			return false;

		const size_t end = message.find('"', pos + 1);
		if (end == std::string_view::npos)
			return false;

		value.clear();
		for (size_t i = pos + 1; i < end; ++i) {
			if (message[i] == '\\' && i + 1 < end) {
				++i;
				switch (message[i]) {
				case 'n':
					value.push_back('\n');
					break;
				case 'r':
					value.push_back('\r');
					break;
				case 't':
					value.push_back('\t');
					break;
				default:
					value.push_back(message[i]);
					break;
				}
			}
			else {
				value.push_back(message[i]);
			}
		}
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

	bool EditorApiServer::extractInteger(std::string_view message, std::string_view key, int32_t& value)
	{
		float numeric = 0;
		if (!extractNumber(message, key, numeric))
			return false;
		value = static_cast<int32_t>(numeric);
		return true;
	}

	std::string EditorApiServer::escapeJson(std::string_view value)
	{
		std::string escaped;
		escaped.reserve(value.size());
		for (const char ch : value) {
			switch (ch) {
			case '\\':
				escaped += "\\\\";
				break;
			case '"':
				escaped += "\\\"";
				break;
			case '\n':
				escaped += "\\n";
				break;
			case '\r':
				escaped += "\\r";
				break;
			case '\t':
				escaped += "\\t";
				break;
			default:
				escaped.push_back(ch);
				break;
			}
		}
		return escaped;
	}
}
