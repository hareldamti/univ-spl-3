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
        //mutex object to protect userCommands
        mutex userCommands_mutex;
    
    public:
        ClientIO();
        

    //thread 1 task - resolving and processing user command input
	void registerInputs();
    //thread 2 task - communicating with server
    void displayResponses();
};