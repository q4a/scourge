#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <iostream>
#include <queue>
#include <map>

using namespace std;

class CommandInterpreter {
 public:
  virtual void chat(char *message) = 0;
  virtual void logout() = 0;
  virtual void ping(int frame) = 0;
  virtual void handleUnknownMessage() = 0;
};

class Commands {
 private:
  CommandInterpreter *ci;
 public:
  Commands(CommandInterpreter *ci);
  ~Commands();

  void interpret(char *rawMessage);
};

#endif
