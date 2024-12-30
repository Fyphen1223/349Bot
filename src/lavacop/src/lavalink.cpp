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
		printf("[lavacop:WebSocket] WebSocket URL is empty\n");
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
	url = std::move(other.url);
	fetchUrl = std::move(other.fetchUrl);
	password = std::move(other.password);
	botId = std::move(other.botId);
	userAgent = std::move(other.userAgent);

	Players = std::move(other.Players);

	readyCallbacks = std::move(other.readyCallbacks);
	closeCallbacks = std::move(other.closeCallbacks);
	messageCallbacks = std::move(other.messageCallbacks);

	sendPayload = std::move(other.sendPayload);
}

LavaLink &LavaLink::operator=(LavaLink &&other) noexcept {
	if (this != &other) {
		url = std::move(other.url);
		fetchUrl = std::move(other.fetchUrl);
		password = std::move(other.password);
		botId = std::move(other.botId);
		userAgent = std::move(other.userAgent);

		Players = std::move(other.Players);

		readyCallbacks = std::move(other.readyCallbacks);
		closeCallbacks = std::move(other.closeCallbacks);
		messageCallbacks = std::move(other.messageCallbacks);

		sendPayload = std::move(other.sendPayload);
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

void LavaLink::onClose(const std::function<void()> &callback) {
	closeCallbacks.push_back(callback);
}

void LavaLink::onMessage(const std::function<void(const std::string data)> &callback) {
	messageCallbacks.push_back(callback);
}

void LavaLink::removeAllReadyListeners() {
	readyCallbacks.clear();
}

void LavaLink::removeAllCloseListeners() {
	closeCallbacks.clear();
}

void LavaLink::removeAllMessageListeners() {
	messageCallbacks.clear();
}

void LavaLink::emitReady() {
	for (auto &callback: readyCallbacks) {
		callback();
	}
}

void LavaLink::emitClose() {
	for (auto &callback: closeCallbacks) {
		callback();
	}
}

void LavaLink::emitMessage(const std::string &msg) {
	for (auto &callback: messageCallbacks) {
		callback(msg);
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
	ws.onMessage([this](const std::string &msg) {
		nlohmann::json data = nlohmann::json::parse(msg);
		if (data["op"] == "ready") {
			sessionId = data["sessionId"].get<std::string>();
			emitReady();
		}
		emitMessage(msg);
	});
	ws.open(url, headers);
}

bool LavaLink::isReachable() {
	auto resp = requests::get((url + "/").c_str());
	if (resp == nullptr) {
		return false;
	}
	return true;
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

void LavaLink::join(const std::string &guildId, const std::string &channelId, const bool &selfDeaf, const bool &selfMute) {
	nlohmann::json payload = {
		{"op", 4},
		{"d", {{"guild_id", guildId}, {"channel_id", channelId}, {"self_mute", selfMute}, {"self_deaf", selfDeaf}}}};

	std::string payload_str = payload.dump();
	if (sendPayload) {
		sendPayload(guildId, payload_str);
		Players[guildId] = Player(PlayerConfig{.sendPayload = sendPayload, .lavalink = this, .guildId = guildId});
	} else {
		printf("sendPayload is not set.\n");
	}
}

void LavaLink::leave(const std::string &guildId) {
	//TODO
}

void LavaLink::setSendPayload(const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload) {
	this->sendPayload = sendPayload;
}

Player::Player(const PlayerConfig &config, const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload, const std::string &guildId)
	: config(config), sendPayload(sendPayload), guildId(guildId) {
}

Player::Player(Player &&other) noexcept
	: config(std::move(other.config)) {
	config = std::move(other.config);
	Node = std::move(other.Node);
	sendPayload = std::move(other.sendPayload);
	stateCallbacks = std::move(other.stateCallbacks);
	playerUpdateCallbacks = std::move(other.playerUpdateCallbacks);
	trackStartCallbacks = std::move(other.trackStartCallbacks);
	trackEndCallbacks = std::move(other.trackEndCallbacks);
	trackExceptionCallbacks = std::move(other.trackExceptionCallbacks);
	trackStuckCallbacks = std::move(other.trackStuckCallbacks);
	webSocketClosedCallbacks = std::move(other.webSocketClosedCallbacks);
}

Player &Player::operator=(Player &&other) noexcept {
	if (this != &other) {
		config = std::move(other.config);
		Node = std::move(other.Node);
		sendPayload = std::move(other.sendPayload);
		stateCallbacks = std::move(other.stateCallbacks);
		playerUpdateCallbacks = std::move(other.playerUpdateCallbacks);
		trackStartCallbacks = std::move(other.trackStartCallbacks);
		trackEndCallbacks = std::move(other.trackEndCallbacks);
		trackExceptionCallbacks = std::move(other.trackExceptionCallbacks);
		trackStuckCallbacks = std::move(other.trackStuckCallbacks);
		webSocketClosedCallbacks = std::move(other.webSocketClosedCallbacks);
	}
	return *this;
}

nlohmann::json Player::update(const nlohmann::json &data, const bool noReplace) {
	if (Node.sessionId.empty()) {
		printf("[lavacop:lavalink]sessionId is empty.\n");
		return nlohmann::json();
	}

	http_headers headers;
	headers["Authorization"] = Node.password;

	const std::string d = data.dump();
	auto resp = requests::patch((Node.fetchUrl + "/v4/sessions/" + Node.sessionId + "/players/" + guildId + "?noReplace=" + (noReplace ? "true" : "false")).c_str(), d, headers);
	if (resp == nullptr) {
		return nlohmann::json();
	}
	return nlohmann::json::parse(resp->body);
}

//void Player::handleEvents()