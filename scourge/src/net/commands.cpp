#include "commands.h"

Commands::Commands(CommandInterpreter *ci) {
  this->ci = ci;
  this->lastGameFrameReceived = 0;
  this->lastCommand = -1;
}

Commands::~Commands() {
}

void Commands::interpret(char *rawMessage) {
  if(!strncmp(rawMessage, "CHAT,", 5)) {
    lastCommand = CHAT;
    ci->chat(rawMessage + 5);
  } else if(!strncmp(rawMessage, "LOGOUT,", 7)) {
    lastCommand = LOGOUT;
    ci->logout();
  } else if(!strncmp(rawMessage, "PING,", 5)) {
    lastCommand = PING;
    ci->ping(atoi(rawMessage + 5));
  } else if(!strncmp(rawMessage, "STATE,", 6)) {
    lastCommand = STATE;
    char *p = strchr(rawMessage + 6, ',');
    lastGameFrameReceived = atoi(rawMessage + 6);
    ci->processGameState(lastGameFrameReceived, p);        
  } else if(!strncmp(rawMessage, "CLOSING", 7)) {
    lastCommand = CLOSING;
    ci->serverClosing();
  } else {
    lastCommand = -1;
    cerr << "* Bad request." << endl;
    ci->handleUnknownMessage();
  }
}

void Commands::buildGameState(char *buff, int frame, char *state) {
  sprintf(buff, "STATE,%d,%s", frame, state);
}

void Commands::buildChat(char *buff, char *username, char *message) {
  sprintf(buff, "CHAT,%s> %s", username, message);
}

void Commands::buildLogin(char *buff, char *username) {
  sprintf(buff, "LOGIN,%s", username);
}

void Commands::buildPing(char *buff) {
  sprintf(buff, "PING,%d", getLastGameFrameReceived());
}

void Commands::buildCharacter(char *buff, char *bytes, int length) {
  strcpy(buff, "CHAR");
  memcpy(buff + 4, bytes, length);
  buff[4 + length] = 0;
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

void TestCommandInterpreter::serverClosing() {
  cout << "Server closing." << endl;
}


