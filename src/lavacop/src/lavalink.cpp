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
	return;
}

void WS::onClose(const std::function<void()> &callback) {
	on_close_callbacks.push_back(callback);
	return;
}

void WS::onMessage(const std::function<void(const std::string &)> &callback) {
	on_message_callbacks.push_back(callback);
	return;
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
	//return; //for testing
}

void WS::close() {
	if (!is_open) {
		return;
	}
	ws.close();
	return;
}

void WS::setPingInterval(int interval) {
	if (!is_open) {
		return;
	}
	ws.setPingInterval(interval);
	return;
}

void WS::send(const std::string &msg) {
	if (!is_open) {
		return;
	}
	ws.send(msg);
	return;
}

void WS::removeAllMessageListeners() {
	on_message_callbacks.clear();
	return;
}

void WS::removeAllOpenListeners() {
	on_open_callbacks.clear();
	return;
}

void WS::removeAllCloseListeners() {
	on_close_callbacks.clear();
	return;
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
	return;
}

void LavaLink::send(const std::string &msg) {
	ws.send(msg);
	return;
}

void LavaLink::onReady(const std::function<void()> &callback) {
	readyCallbacks.push_back(callback);
	return;
}

void LavaLink::onClose(const std::function<void()> &callback) {
	closeCallbacks.push_back(callback);
	return;
}

void LavaLink::onMessage(const std::function<void(const std::string data)> &callback) {
	messageCallbacks.push_back(callback);
	return;
}

void LavaLink::removeAllReadyListeners() {
	readyCallbacks.clear();
	return;
}

void LavaLink::removeAllCloseListeners() {
	closeCallbacks.clear();
	return;
}

void LavaLink::removeAllMessageListeners() {
	messageCallbacks.clear();
	return;
}

void LavaLink::emitReady() {
	for (auto &callback: readyCallbacks) {
		callback();
	}
	return;
}

void LavaLink::emitClose() {
	for (auto &callback: closeCallbacks) {
		callback();
	}
	return;
}

void LavaLink::emitMessage(const std::string &msg) {
	for (auto &callback: messageCallbacks) {
		callback(msg);
	}
	return;
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
		if (data["op"] == "playerUpdate") {
			const std::string guildId = data["guildId"];
			if (Players.find(guildId) != Players.end()) {
				Players[guildId].handleLavaLinkEvents(msg);
			}
		}
	});
	ws.open(url, headers);
	return;
}

bool LavaLink::isReachable() {
	auto resp = requests::get((url + "/").c_str());
	if (resp == nullptr) {
		return false;
	}
	return true;
}

std::string urlEncode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (char c: value) {
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
		} else {
			escaped << '%' << std::setw(2) << int((unsigned char) c);
		}
	}
	return escaped.str();
}

nlohmann::json LavaLink::loadTracks(const std::string &identifier) {
	http_headers headers;
	headers["Authorization"] = password;
	auto resp = requests::get((fetchUrl + "/v4/loadtracks?identifier=" + urlEncode(identifier)).c_str(), headers);
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
	throw std::runtime_error("Player not found for guildId: " + guildId);
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
		return nlohmann::json();
	}
	http_headers headers;
	headers["Authorization"] = Node->password;
	headers["Content-Type"] = "application/json";

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
			playing = true;
			for (auto &callback: trackStartCallbacks) {
				callback(data);
			}
		}
		if (raw["type"] == "TrackEndEvent") {
			playing = false;
			for (auto &callback: trackEndCallbacks) {
				callback(data);
			}
		}
		if (raw["type"] == "TrackExceptionEvent") {
			playing = false;
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
			playing = false;
			for (auto &callback: webSocketClosedCallbacks) {
				callback(data);
			}
		}
	}
	if (raw["op"] == "playerUpdate") {
		ping = raw["state"]["ping"].get<int>();
		position = raw["state"]["position"].get<int>();
		connected = raw["state"]["connected"].get<bool>();
		for (auto &callback: playerUpdateCallbacks) {
			callback(data);
		}
	}
}

void Player::get() {
	if (Node->sessionId.empty()) {
		printf("[lavacop:Player] sessionId is empty.\n");
		return;
	}
	http_headers headers;
	headers["Authorization"] = Node->password;
	headers["Content-Type"] = "application/json";
	auto resp = requests::get((Node->fetchUrl + "/v4/sessions/" + Node->sessionId + "/players/" + this->guildId).c_str(), headers);
	if (resp == nullptr) {
		return;
	}

	nlohmann::json data = nlohmann::json::parse(resp->body);

	if (data["state"]["position"] != nullptr) {
		position = data["state"]["position"].get<int>();
	}
	if (data["state"]["paused"] != nullptr) {
		paused = data["state"]["paused"].get<bool>();
	}
	if (data["state"]["ping"] != nullptr) {
		int p = data["state"]["ping"];
		if (p == -1) {
			ping = 0;
		} else {
			ping = p;
		}
	}
	if (data["track"] != nullptr) {
		playing = true;
	}
	if (data["volume"] != nullptr) {
		volumeLevel = data["volume"];
	}

	return;
}

void Player::connect() {
	nlohmann::json data = {
		{"voice", {{"token", this->token}, {"endpoint", this->endpoint}, {"sessionId", this->sessionId}}}};
	update(data);
	return;
}

void Player::play(const std::string &track, const int startTime, const int endTime, const bool noReplace) {
	nlohmann::json trackData = {
		{"track", {{"encoded", track}}}};
	update(trackData, noReplace);
	return;
}

void Player::pause() {
	nlohmann::json data = {
		{"paused", true}};
	paused = true;
	update(data);
	return;
}

void Player::resume() {
	nlohmann::json data = {
		{"paused", false}};
	paused = false;
	update(data);
	return;
}

void Player::stop() {
	nlohmann::json data = {
		{"track", {{"encoded", nullptr}}}};
	update(data);
	return;
}

void Player::volume(const int volume) {
	nlohmann::json data = {
		{"volume", volume}};
	volumeLevel = volume;
	update(data);
	return;
}

void Player::seek(const int position) {
	nlohmann::json data = {
		{"position", position}};
	this->position = position;
	update(data);
	return;
}
