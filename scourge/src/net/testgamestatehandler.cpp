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

// the consumer
void TestGameStateHandler::consumeGameState(int frame, char *state) {
  //  if(!(frame % 5000))
    cout << "*** Current game state: frame=" << frame << " " << state << endl;
}

