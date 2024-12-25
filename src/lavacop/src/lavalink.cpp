#include "hv/WebSocketClient.h"
#include <atomic>
#include <chrono>
#include <thread>
using namespace hv;

class WS {
  public:
	WS()
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
			is_open = false;
			loop.stop();
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

	void open(const std::string &url) {
		if (is_open) {
			return;
		}
		ws.open(url.c_str());
		loop.run();
	}

	void setPingInterval(int interval) {
		if (!is_open) {
			return;
		}
		ws.setPingInterval(interval);
	}

	void send(const std::string &msg) {
		if (!is_open) {
			printf("WebSocket is not open\n");
			return;
		}
		printf("send: %s\n", msg.c_str());
		ws.send(msg);
	}

	void close() {
		if (!is_open) {
			return;
		}
		ws.close();
	}

	void onMessage(const std::function<void(const std::string &)> &callback) {
		on_message_callbacks.push_back(callback);
	}

	void onOpen(const std::function<void()> &callback) {
		on_open_callbacks.push_back(callback);
	}

	void onClose(const std::function<void()> &callback) {
		on_close_callbacks.push_back(callback);
	}

	/*
	void removeMessageListener(const std::function<void(const std::string &)> &callback) {
		on_message_callbacks.erase(std::remove(on_message_callbacks.begin(), on_message_callbacks.end(), callback), on_message_callbacks.end());
	}
	void removeOpenListener(const std::function<void()> &callback) {
		on_open_callbacks.erase(std::remove(on_open_callbacks.begin(), on_open_callbacks.end(), callback), on_open_callbacks.end());
	}
	void removeCloseListener(const std::function<void()> &callback) {
		on_close_callbacks.erase(std::remove(on_close_callbacks.begin(), on_close_callbacks.end(), callback), on_close_callbacks.end());
	}
	*/

	void removeAllMessageListeners() {
		on_message_callbacks.clear();
	}
	void removeAllOpenListeners() {
		on_open_callbacks.clear();
	}
	void removeAllCloseListeners() {
		on_close_callbacks.clear();
	}

  private:
	hv::WebSocketClient ws;
	hv::EventLoop loop;
	std::atomic<bool> is_open;
	reconn_setting_t reconn;
	std::vector<std::function<void(const std::string &)>> on_message_callbacks;
	std::vector<std::function<void()>> on_open_callbacks;
	std::vector<std::function<void()>> on_close_callbacks;
};

/*
void startWebSocket() {
	WS ws;
	ws.onMessage([](const std::string &msg) {
		std::cout << "onMessage: " << msg << std::endl;
	});
	ws.onOpen([&ws]() {
		ws.send("Hello, WebSocket!");
	});
	ws.open("wss://echo.websocket.org:443");
}
*/

class LavaLink {
  public:
	LavaLink(const std::string &url)
		: url(url) {
		ws.onMessage([this](const std::string &msg) {
			printf("onMessage: %s\n", msg.c_str());
		});
		ws.onOpen([this]() {
			printf("onOpen\n");
			ws.send("Hello, WebSocket!");
		});
		ws.onClose([this]() {
			printf("onClose\n");
		});
	}

	void open() {
		ws.open(url);
	}

	void close() {
		ws.close();
	}

	void send(const std::string &msg) {
		ws.send(msg);
	}

  private:
	std::string url;
	WS ws;
};