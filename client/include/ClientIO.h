#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <map>

#include "ConnectionHandler.h"
#include "frame.h"
#include "event.h"

using namespace std;

enum ClientState {
    AwaitingLogin,
    AwaitingConnected,
    Connected,
    AwaitingDisconnection,
    Disconnected
};

class ClientIO{
    public:
        ConnectionHandler connectionHandler;

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
        int generateNewSubId();
        int generateNewReceiptId();
        bool startConnection();

        //thread 1 task - resolving and processing user command input
        void sendRequests();

        void processInput(string input);

        void sendStompFrame(Frame& frame);

        void processMessages();
};

