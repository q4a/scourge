#include "clientinfo.h"

ClientInfo::ClientInfo(Server *server, TCPsocket socket, int id, char *username) {
  this->server = server;
  this->socket = socket;
  this->dead = false;
  this->id = id;
  this->username = username;
  this->commands = new Commands(this);

  // init the lag
  aveLag = 0.0f;
  totalLag = 0;
  lastLagCheck = 0;

  // start the client thread
  threadRunning = true;
  mutex = SDL_CreateMutex();
  cond = SDL_CreateCond();
  thread = SDL_CreateThread(clientInfoLoop, this);
}

ClientInfo::~ClientInfo() {
  delete commands;
  // stop the thread
  threadRunning = false;
  // wake up the thread
  handleRequestAsync();
  int status;
  SDL_WaitThread(thread, &status);
  // kill the mutex, cond
  SDL_DestroyMutex(mutex);
  SDL_DestroyCond(cond);
  // close the socket
  SDLNet_TCP_Close(socket);
  // misc. other stuff
  free(username);
  // empty message queue
  while(!messageQueue.size()) {
    Message *message = messageQueue.front();
    messageQueue.pop();
    delete message;
  }
}

char *ClientInfo::describe() {
  sprintf(desc, "id=%d name=%s", id, username);
  return desc;
}

void ClientInfo::handleRequestAsync() {
  // wake up the thread
  if(SDL_CondSignal(cond) == -1) {
    cerr << "Couldn't signal condition." << endl;
    exit(6);
  }  
}

void ClientInfo::chat(char *message) {
  // FIXME: prepend "from" to the message
  server->sendToAllTCP(message);
}

void ClientInfo::logout() {
  cerr << "\t* logout: " << describe() << endl;
  dead = true;
}

void ClientInfo::ping(int frame) {
  // keep track of client's lag
  updateLag(frame);
}

void ClientInfo::handleUnknownMessage() {
  // bad request; kill client
  dead = true;
}

void ClientInfo::sendMessageAsync(char *message) {
  // wake up the thread
  if(message && !dead) {

    // lock the mutex
    if(SDL_mutexP(mutex) == -1) {
      cerr << "Couldn't lock mutex." << endl;
      exit(7);
    }

    // put the message on the queue
    messageQueue.push(new Message(strdup(message)));

    // unlock the mutex
    if(SDL_mutexV(mutex) == -1) {
      cerr << "Couldn't unlock mutex." << endl;
      exit(7);
    }

    if(SDL_CondSignal(cond) == -1) {
      cerr << "Couldn't signal condition." << endl;
      exit(6);
    }
  }
}

void ClientInfo::receiveTCP() {
  // read from the socket
  char *text = NULL;
  if(TCPUtil::receive(socket, &text)) {
    commands->interpret(text);
  }
  free(text);
}

void ClientInfo::sendTCP(Message *message) {
  if(message) {
    if(!TCPUtil::send(socket, message->message)) {
      cerr << "* Can't send TCP to client: " << describe() << endl;
      dead = true;
    }
  }
}

int clientInfoLoop(void *data) {
  bool runAgain = false;
  ClientInfo *clientInfo = (ClientInfo*)data;
  while(clientInfo->isThreadRunning()) {
    // lock the mutex
    if(SDL_mutexP(clientInfo->getMutex()) == -1) {
      cerr << "Couldn't lock mutex." << endl;
      exit(7);
    }    
    // then wait for a signal to start again
    if(!runAgain && 
       SDL_CondWait(clientInfo->getCond(), clientInfo->getMutex()) == -1) {
      cerr << "Couldn't wait on condition." << endl;
      exit(7);
    }
    // remove a message from the queue
    Message *message = NULL;
    if(clientInfo->getMessageQueue()->size()) {
      // do I need to copy the message?
      message = clientInfo->getMessageQueue()->front();
      clientInfo->getMessageQueue()->pop();
      runAgain = (clientInfo->getMessageQueue()->size() > 0 ? true : false);
    } else {
      runAgain = false;
    }
    // unlock the mutex
    if(SDL_mutexV(clientInfo->getMutex()) == -1) {
      cerr << "Couldn't unlock mutex." << endl;
      exit(7);
    }

    // send message if any
    if(message) {
      clientInfo->sendTCP(message);
      delete message;
    }

    // handle the request if any
    if(SDLNet_SocketReady(clientInfo->socket)) {
      clientInfo->receiveTCP();
    }
  }
  return 0;
}

void ClientInfo::setLagTimer(int frame, Uint32 n) {
  if((int)lagMap.size() > MAX_SCHEDULED_LAG_MESSAGES) {
    dead = true;
  } else {
    lagMap[frame] = n;
  }
}

Uint32 ClientInfo::updateLag(int frame) {
  if(lagMap.find(frame) != lagMap.end()) {
    Uint32 n = lagMap[frame];
    
    // FIXME: do this more efficiently
    // remove older frames
    for(map<int,Uint32>::iterator i=lagMap.begin(); i!=lagMap.end(); ++i) {
      int f = i->first;
      //      Uint32 t = i->second;
      if(f <= frame) lagMap.erase(i);
    }

    // compute the lag
    Uint32 t = SDL_GetTicks();
    Uint32 lag = t - n;
    totalLag += lag;
    if(t - lastLagCheck > 5000) {
      aveLag = (float)totalLag / (float)(t - lastLagCheck);
      totalLag = 0;
      lastLagCheck = t;
      cerr << "Avg lag for " << describe() << " is " << ((float)aveLag / 1000.0f) << " sec." << endl;
    }

    return n;
  }
  return (Uint32)0;
}

Message::Message(char *message) {
  this->message = message;
}

Message::~Message() {
  free(message);
}
