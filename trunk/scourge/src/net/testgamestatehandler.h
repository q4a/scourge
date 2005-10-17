#ifndef TEST_GAME_STATE_HANDLER_H
#define TEST_GAME_STATE_HANDLER_H

#include "../constants.h"
#include "gamestatehandler.h"

class TestGameStateHandler : public GameStateHandler {
 private:
  char state[1024];

 public:
  TestGameStateHandler();
  virtual ~TestGameStateHandler();

  // the producer
  char *getGameState();
  
};

#endif
