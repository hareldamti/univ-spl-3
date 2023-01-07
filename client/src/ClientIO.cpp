#include "../include/ClientIO.h"
#include "../include/ConnectionHandler.h"
#include <iostream>
#include <mutex>
using namespace std;


class ClientIO {
	bool terminate;
	vector<string> userCommands;
    mutex userCommands_mutex;

	ClientIO() :terminate(false),userCommands(), userCommands_mutex(){}
    

	void registerInputs() {
		while (!terminate) {
			const short bufsize = 1024;
			char buf[bufsize];
			cin.getline(buf, bufsize);
			string line(buf);
			int len=line.length();
		}
	}

	void displayResponses(ConnectionHandler& ch) {
		while (!terminate) {
			// lockguard - mutex - lock queue - read inputs - send - wait for response - process = display
            string currentCommand;
            {
                lock_guard<mutex> lock(userCommands_mutex);
                if(!userCommands.empty()){
                    currentCommand = userCommands.back();
                    userCommands.pop_back();
                }
            }

            if(!currentCommand.empty()){

                if (!ch.sendLine(currentCommand)) {
                    cout << "Disconnected. Exiting...\n" << endl;
                    terminate = true;
                }
            }
            string serverResponse;
            if(!ch.getLine(serverResponse)){
                //maybe there is a better solution
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                terminate = true;
            }

            //handle serverResponse


		}
	}


};