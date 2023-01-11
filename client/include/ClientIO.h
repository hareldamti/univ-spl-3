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
        mutex stateLock, eventsLock;
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

        // Rule of 5
        ~ClientIO();
        ClientIO(const ClientIO& other);
        ClientIO(const ClientIO&& other);
        ClientIO& operator=(const ClientIO& other);
        ClientIO& operator=(const ClientIO&& other);

        // Getters & setters
        int generateNewSubId();
        int generateNewReceiptId();
        bool startConnection();
        void setState(ClientState newState);
        boolean compareState(ClientState otherState);

        //Threads tasks
        void sendRequests();
        void processInput(string input);
        void sendStompFrame(Frame& frame);
        void processMessages();
};

