#ifndef GAME_STATE_HANDLER_H
#define GAME_STATE_HANDLER_H

#include "../constants.h"

using namespace std;

class GameStateHandler {
 public:
  GameStateHandler();
  virtual ~GameStateHandler();

  // the producer
  virtual char *getGameState() = 0;
  
  // the consumer
  virtual void consumeGameState(int frame, char *state) = 0;
};

#endif
