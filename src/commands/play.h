#ifndef PLAY_H
#define PLAY_H

#include <iostream>
#include <string>

class PlayCommand {
  public:
	void ping() {
		std::cout << "Pong!" << std::endl;
	}
};

#endif// PLAY_H