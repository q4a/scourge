#include "testgamestatehandler.h"

TestGameStateHandler::TestGameStateHandler() {
  strcpy(state, "xyzabcdef");
}

TestGameStateHandler::~TestGameStateHandler() {
}

// the producer
char *TestGameStateHandler::getGameState() {
  return state;
}
