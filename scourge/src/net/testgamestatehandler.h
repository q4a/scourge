#ifndef TEST_GAME_STATE_HANDLER_H
#define TEST_GAME_STATE_HANDLER_H

#include <string>
#include <iostream>
#include "gamestatehandler.h"

using namespace std;

class TestGameStateHandler : public GameStateHandler {
 private:
  char state[1024];

 public:
  TestGameStateHandler();
  virtual ~TestGameStateHandler();

  // the producer
  char *getGameState();
  
  // the consumer
  void consumeGameState(int frame, char *state);
};

#endif
