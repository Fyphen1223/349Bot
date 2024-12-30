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

	sessionId = std::move(other.sessionId);
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

		sessionId = std::move(other.sessionId);
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
		if (data["op"] == "event") {
			const std::string guildId = data["guildId"];
			if (Players.find(guildId) != Players.end()) {
				Players[guildId].handleLavaLinkEvents(msg);
			}
		}
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

void LavaLink::handleRaw(const nlohmann::json &raw) {
	if (Players.find(raw["d"]["guild_id"]) != Players.end()) {
		Players[raw["d"]["guild_id"]].handleEvents(raw);
	}
}

Player &LavaLink::getPlayer(const std::string &guildId) {
	if (Players.find(guildId) != Players.end()) {
		return Players[guildId];
	}
	return Players[guildId];
}


Player::Player(const PlayerConfig &config)
	: config(config), Node(config.lavalink), sendPayload(config.sendPayload), guildId(config.guildId) {
}

Player::Player(Player &&other) noexcept
	: config(std::move(other.config)), Node(other.Node), guildId(std::move(other.guildId)), sendPayload(std::move(other.sendPayload)) {
	Node = std::move(other.Node);
	sendPayload = std::move(other.sendPayload);
	stateCallbacks = std::move(other.stateCallbacks);
	playerUpdateCallbacks = std::move(other.playerUpdateCallbacks);
	trackStartCallbacks = std::move(other.trackStartCallbacks);
	trackEndCallbacks = std::move(other.trackEndCallbacks);
	trackExceptionCallbacks = std::move(other.trackExceptionCallbacks);
	trackStuckCallbacks = std::move(other.trackStuckCallbacks);
	webSocketClosedCallbacks = std::move(other.webSocketClosedCallbacks);
	token = std::move(other.token);
	endpoint = std::move(other.endpoint);
	sessionId = std::move(other.sessionId);
	guildId = std::move(other.guildId);
}

Player &Player::operator=(Player &&other) noexcept {
	if (this != &other) {
		Node = std::move(other.Node);
		sendPayload = std::move(other.sendPayload);
		stateCallbacks = std::move(other.stateCallbacks);
		playerUpdateCallbacks = std::move(other.playerUpdateCallbacks);
		trackStartCallbacks = std::move(other.trackStartCallbacks);
		trackEndCallbacks = std::move(other.trackEndCallbacks);
		trackExceptionCallbacks = std::move(other.trackExceptionCallbacks);
		trackStuckCallbacks = std::move(other.trackStuckCallbacks);
		webSocketClosedCallbacks = std::move(other.webSocketClosedCallbacks);
		token = std::move(other.token);
		endpoint = std::move(other.endpoint);
		sessionId = std::move(other.sessionId);
		guildId = std::move(other.guildId);
	}
	return *this;
}

nlohmann::json Player::update(const nlohmann::json &data, const bool noReplace) {
	if (Node->sessionId.empty()) {
		printf("[lavacop:lavalink] sessionId is empty.\n");
		return nlohmann::json();
	}
	http_headers headers;
	headers["Authorization"] = Node->password;

	const std::string d = data.dump();
	auto resp = requests::patch((Node->fetchUrl + "/v4/sessions/" + Node->sessionId + "/players/" + this->guildId + "?noReplace=" + (noReplace ? "true" : "false")).c_str(), d, headers);
	if (resp == nullptr) {
		return nlohmann::json();
	}
	return nlohmann::json::parse(resp->body);
}

void Player::handleEvents(const nlohmann::json &data) {
	if (data["t"] == "VOICE_SERVER_UPDATE") {
		endpoint = data["d"]["endpoint"].get<std::string>();
		token = data["d"]["token"].get<std::string>();
		if (!sessionId.empty()) {
			connect();
		}
	} else if (data["t"] == "VOICE_STATE_UPDATE") {
		sessionId = data["d"]["session_id"].get<std::string>();
		if (!token.empty()) {
			connect();
		}
	}
}

void Player::handleLavaLinkEvents(std::string data) {
	nlohmann::json raw = nlohmann::json::parse(data);
	if (raw["op"] == "event") {
		if (raw["type"] == "TrackStartEvent") {
			for (auto &callback: trackStartCallbacks) {
				callback(data);
			}
		}
		if (raw["type"] == "TrackEndEvent") {
			for (auto &callback: trackEndCallbacks) {
				callback(data);
			}
		}
		if (raw["type"] == "TrackExceptionEvent") {
			for (auto &callback: trackExceptionCallbacks) {
				callback(data);
			}
		}
		if (raw["type"] == "TrackStuckEvent") {
			for (auto &callback: trackStuckCallbacks) {
				callback(data);
			}
		}
		if (raw["type"] == "WebSocketClosedEvent") {
			for (auto &callback: webSocketClosedCallbacks) {
				callback(data);
			}
		}
	}
}

void Player::connect() {
	nlohmann::json data = {
		{"voice", {{"token", this->token}, {"endpoint", this->endpoint}, {"sessionId", this->sessionId}}}};
	update(data);
}

void Player::play(const std::string &track, const int startTime, const int endTime, const bool noReplace) {
	nlohmann::json trackData = {
		{"track", {{"encoded", track}}}};
	update(trackData), noReplace;
}

void Player::pause() {
	nlohmann::json data = {
		{"paused", true}};
	paused = true;
	update(data);
}

void Player::resume() {
	nlohmann::json data = {
		{"paused", false}};
	paused = false;
	update(data);
}

void Player::stop() {
	nlohmann::json data = {
		{"track", {{"encoded", nullptr}}}};
	update(data);
}

void Player::volume(const int volume) {
	nlohmann::json data = {
		{"volume", volume}};
	volumeLevel = volume;
	update(data);
}

void Player::seek(const int position) {
	nlohmann::json data = {
		{"position", position}};
	this->position = position;
	update(data);
}
