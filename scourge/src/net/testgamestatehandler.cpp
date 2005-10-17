#include "testgamestatehandler.h"

using namespace std;

TestGameStateHandler::TestGameStateHandler() {
  strcpy(state, "xyzabcdef");
}

TestGameStateHandler::~TestGameStateHandler() {
}

// the producer
char *TestGameStateHandler::getGameState() {
  return state;
}
