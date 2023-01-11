#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <map>
#include "ConnectionHandler.h"
#include "../include/event.h"

using namespace std;
class Frame;

enum ClientState {
    AwaitingLogin,
    AwaitingConnected,
    Connected,
    AwaitingDisconnected,
    Disconnected
};

class ClientIO{
    public:
        ConnectionHandler* connectionHandler;
        int nextStateReceipt;
        ClientState state;
        // flag representing connection with server

        string username;
        //manages username-subscriptionId mappings
        map<string, int> subscriptions;
        //uniqueIds factory
        int subIdCounter, msgIdCounter;
        map<string, map<string, vector<Event>>> totalEvents;
    public:
        ClientIO();
        ~ClientIO();
        ClientIO(const ClientIO& other);
        ClientIO(const ClientIO&& other);
        ClientIO& operator=(const ClientIO& other);
        ClientIO& operator=(const ClientIO&& other);
        int generateNewSubId();
        int generateNewReceiptId();
        bool startConnection();

        //thread 1 task - resolving and processing user command input
        void sendRequests();
        void processInput(string input);
        void sendStompFrame(Frame& frame);
        void processMessages();
};

