#ifndef COMMANDS_H
#define COMMANDS_H

#include "../constants.h"

using namespace std;

class CommandInterpreter {
 public:
  virtual void chat(char *message) = 0;
  virtual void logout() = 0;
  virtual void ping(int frame) = 0;
  virtual void processGameState(int frame, char *p) = 0;
  virtual void handleUnknownMessage() = 0;
};

class TestCommandInterpreter : public CommandInterpreter {
public:
  TestCommandInterpreter();
  virtual ~TestCommandInterpreter();
  void chat(char *message);
  void logout();
  void ping(int frame);
  void processGameState(int frame, char *p);
  void handleUnknownMessage();
};

class Commands {
 private:
  CommandInterpreter *ci;
  int lastGameFrameReceived;
 public:
  Commands(CommandInterpreter *ci);
  ~Commands();

  inline int getLastGameFrameReceived() { return lastGameFrameReceived; }

  void interpret(char *rawMessage);
};

#endif
