#ifndef COMMANDS_H
#define COMMANDS_H

#include "../constants.h"
#include "../persist.h"

using namespace std;

class CommandInterpreter {
 public:
  virtual void chat(char *message) = 0;
  virtual void logout() = 0;
  virtual void ping(int frame) = 0;
  virtual void processGameState(int frame, char *p) = 0;
  virtual void handleUnknownMessage() = 0;
  virtual void serverClosing() = 0;
  virtual void character(char *bytes, int length) = 0;
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
  void serverClosing();
  void character(char *bytes, int length);
};

class Commands {
 private:
  CommandInterpreter *ci;
  int lastGameFrameReceived;
  int lastCommand;
 public:
   
   enum {
     CHAT=0,
     LOGOUT,
     PING,
     STATE,
     CLOSING,
     CHARACTER,

     // must be last entry
     COMMAND_COUNT
   };

  Commands(CommandInterpreter *ci);
  ~Commands();

  inline int getLastGameFrameReceived() { return lastGameFrameReceived; }
  void interpret(char *rawMessage, int length);
  inline int getLastCommand() { return lastCommand; }

  static void buildGameState(char *buff, int frame, char *state);
  static void buildChat(char *buff, char *username, char *message);
  static void buildLogin(char *buff, char *username);
  // not static!
  void buildPing(char *buff);
  static void buildBytesCharacter(char *buff, int size, char *info, int *messageSize);
};

#endif
