#include "commands.h"

Commands::Commands(CommandInterpreter *ci) {
  this->ci = ci;
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
  } else {
    cerr << "* Bad request." << endl;
    ci->handleUnknownMessage();
  }
}
