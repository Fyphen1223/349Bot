#include "lavalink.h"


//#include <../lavacop.h>
using namespace hv;

WS::WS()
	: is_open(false) {
	ws.onopen = [this]() {
		is_open = true;
		for (const auto &callback: on_open_callbacks) {
			callback();
		}
	};
	ws.onmessage = [this](const std::string &msg) {
		for (const auto &callback: on_message_callbacks) {
			callback(msg);
		}
	};
	ws.onclose = [this]() {
		is_open.store(false);
		for (const auto &callback: on_close_callbacks) {
			callback();
		}
	};
	reconn_setting_init(&reconn);
	reconn.min_delay = 1000;
	reconn.max_delay = 10000;
	reconn.delay_policy = 2;
	ws.setReconnect(&reconn);
}

void WS::onOpen(const std::function<void()> &callback) {
	on_open_callbacks.push_back(callback);
}

void WS::onClose(const std::function<void()> &callback) {
	on_close_callbacks.push_back(callback);
}

void WS::onMessage(const std::function<void(const std::string &)> &callback) {
	on_message_callbacks.push_back(callback);
}

void WS::open(const std::string &url, const http_headers &headers) {
	if (url.empty()) {
		printf("[lavacop] WebSocket URL is empty\n");
	}
	ws.onopen = [this]() {
		is_open = true;
		for (const auto &callback: on_open_callbacks) {
			callback();
		}
	};
	ws.onmessage = [this](const std::string &msg) {
		for (const auto &callback: on_message_callbacks) {
			callback(msg);
		}
	};
	ws.onclose = [this]() {
		is_open = false;
		for (const auto &callback: on_close_callbacks) {
			callback();
		}
	};
	ws.open(url.c_str(), headers);
}

void WS::close() {
	if (!is_open) {
		return;
	}
	ws.close();
}

void WS::setPingInterval(int interval) {
	if (!is_open) {
		return;
	}
	ws.setPingInterval(interval);
}

void WS::send(const std::string &msg) {
	if (!is_open) {
		return;
	}
	ws.send(msg);
}

void WS::removeAllMessageListeners() {
	on_message_callbacks.clear();
}

void WS::removeAllOpenListeners() {
	on_open_callbacks.clear();
}

void WS::removeAllCloseListeners() {
	on_close_callbacks.clear();
}

//WS

LavaLink::LavaLink(const LavaLinkConfig &config, const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload)
	: config(config), sendPayload(sendPayload) {
	url = (config.secure ? "wss://" : "ws://") + config.ip + ":" + config.port + "/v4/websocket";
	fetchUrl = (config.secure ? "https://" : "http://") + config.ip + ":" + config.port;
	password = config.password;
	botId = config.botId;
	userAgent = config.userAgent;
}

LavaLink::LavaLink(LavaLink &&other) noexcept
	: config(std::move(other.config)) {
	password = std::move(other.password);
	botId = std::move(other.botId);
	userAgent = std::move(other.userAgent);
	url = std::move(other.url);
	fetchUrl = std::move(other.fetchUrl);
	sendPayload = std::move(other.sendPayload);
}

LavaLink &LavaLink::operator=(LavaLink &&other) noexcept {
	if (this != &other) {
		config = std::move(other.config);
		password = std::move(other.password);
		botId = std::move(other.botId);
		userAgent = std::move(other.userAgent);
	}
	return *this;
}

void LavaLink::close() {
	ws.close();
}

void LavaLink::send(const std::string &msg) {
	ws.send(msg);
}

void LavaLink::onReady(const std::function<void()> &callback) {
	readyCallbacks.push_back(callback);
}

void LavaLink::onState(const std::function<void(std::string data)> &callback) {
	stateCallbacks.push_back(callback);
}

void LavaLink::onPlayerUpdate(const std::function<void(std::string data)> &callback) {
	playerUpdateCallbacks.push_back(callback);
}

void LavaLink::onClose(const std::function<void()> &callback) {
	closeCallbacks.push_back(callback);
}

void LavaLink::removeAllReadyListeners() {
	readyCallbacks.clear();
}

void LavaLink::removeAllStateListeners() {
	stateCallbacks.clear();
}

void LavaLink::removeAllPlayerUpdateListeners() {
	playerUpdateCallbacks.clear();
}

void LavaLink::removeAllCloseListeners() {
	closeCallbacks.clear();
}

void LavaLink::emitReady() {
	for (auto &callback: readyCallbacks) {
		callback();
	}
}

void LavaLink::emitState(std::string &data) {
	for (auto &callback: stateCallbacks) {
		callback(data);
	}
}

void LavaLink::emitPlayerUpdate(std::string &data) {
	for (auto &callback: playerUpdateCallbacks) {
		callback(data);
	}
}

void LavaLink::emitClose() {
	for (auto &callback: closeCallbacks) {
		callback();
	}
}

void LavaLink::connect() {
	http_headers headers;
	headers["Authorization"] = password;
	headers["User-Id"] = botId;
	headers["Client-Name"] = userAgent;
	ws.onClose([this]() {
		emitClose();
	});
	ws.onOpen([this]() {
		emitReady();
	});
	ws.open(url, headers);
}

std::string encodeIdentifier(const std::string &identifier) {
	std::string encoded;
	for (char c: identifier) {
		if (c == ':') {
			encoded += "%3A";
		} else if (c == '&') {
			encoded += "%26";
		} else if (c == '=') {
			encoded += "%3D";
		} else if (c == '?') {
			encoded += "%3F";
		} else if (c == ',') {
			encoded += "%2C";
		} else if (c == '+') {
			encoded += "%2B";
		} else if (c == ' ') {
			encoded += "%20";
		} else {
			encoded += c;
		}
	}
	return encoded;
}

nlohmann::json LavaLink::loadTracks(const std::string &identifier) {
	http_headers headers;
	headers["Authorization"] = password;
	auto resp = requests::get((fetchUrl + "/v4/loadtracks?identifier=" + encodeIdentifier(identifier)).c_str(), headers);
	if (resp == nullptr) {
		return nlohmann::json();
	}
	return nlohmann::json::parse(resp->body);
}

void LavaLink::join(const dpp::snowflake &guildId, const dpp::snowflake &channelId, const bool &selfDeaf, const bool &selfMute) {
	nlohmann::json payload = {
		{"op", 4},
		{"d", {{"guild_id", guildId}, {"channel_id", channelId}, {"self_mute", selfMute}, {"self_deaf", selfDeaf}}}};

	std::string payload_str = payload.dump();
	if (sendPayload) {
		std::string guildIdStr = std::to_string(guildId);
		sendPayload(guildIdStr, payload_str);
	} else {
		printf("sendPayload is not set.\n");
	}
}

void LavaLink::setSendPayload(const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload) {
	this->sendPayload = sendPayload;
}
