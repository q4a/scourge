#include "commands.h"

Commands::Commands(CommandInterpreter *ci) {
  this->ci = ci;
  this->lastGameFrameReceived = 0;
}

Commands::~Commands() {
}

void Commands::interpret(char *rawMessage) {
  if(!strncmp(rawMessage, "CHAT,", 5)) {
    ci->chat(rawMessage);
  } else if(!strncmp(rawMessage, "LOGOUT,", 7)) {
    ci->logout();
  } else if(!strncmp(rawMessage, "PING,", 5)) {
    ci->ping(atoi(rawMessage + 5));
  } else if(!strncmp(rawMessage, "STATE,", 6)) {
    char *p = strchr(rawMessage + 6, ',');
    lastGameFrameReceived = atoi(rawMessage + 6);
    ci->processGameState(lastGameFrameReceived, p);        
  } else {
    cerr << "* Bad request." << endl;
    ci->handleUnknownMessage();
  }
}

TestCommandInterpreter::TestCommandInterpreter() {
}

TestCommandInterpreter::~TestCommandInterpreter() {
}

void TestCommandInterpreter::chat(char *message) {
  cout << message << endl;
}

void TestCommandInterpreter::logout() {
  cout << "Logout." << endl;
}

void TestCommandInterpreter::ping(int frame) {
  cout << "Ping." << endl;
}

void TestCommandInterpreter::processGameState(int frame, char *p) {
  cout << "Game state: frame=" << frame << " state=" << p << endl;
}

void TestCommandInterpreter::handleUnknownMessage() {
  cout << "Unknown message received." << endl;
}

