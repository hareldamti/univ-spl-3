#pragma once

#include <vector>
#include <string>
#include <mutex>

using namespace std;


class ClientIO{
    public:
        // flag representing connection with server
        bool terminate;
        //list of user commands for the thread to process
        vector<string> userCommands;
        //manages username-subscriptionId mappings
        map<string, int> subscriptions;
        //uniqueIds factory
        int subIdCounter, msgIdCounter;
        //mutex object to protect userCommands
        mutex userCommands_mutex;
    
    public:
        ClientIO();
        int generateNewSubId();
        int generateNewReceiptId();

        //thread 1 task - resolving and processing user command input
        void sendRequests(ConnectionHandler& ch);

        string parseInput(string input);

        //thread 2 task - communicating with server
        void displayMessages(ConnectionHandler& ch);
};

enum ClientState {
    AwaitingLogin,
    AwaitingConnected,
    Connected,
    AwaitingDisconnection,
    Disconnected
}