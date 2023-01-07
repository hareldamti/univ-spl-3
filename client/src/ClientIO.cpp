#include "../include/ClientIO.h"
#include "../include/ConnectionHandler.h"
#include <iostream>
#include <mutex>
#include "frame.cpp"
#include "event.cpp"
using namespace std;


class ClientIO {
	bool terminate;
    int terminateReceipt;
	vector<string> userCommands;
    map<string, int> subscriptions;
    int subIdCounter, msgIdCounter;
	ClientIO(): terminate(false), terminateReceipt(0), userCommands(), subIdCounter(1), msgIdCounter(1){}
    
    int generateNewSubId() {return subIdCounter++; }
    int generateNewReceiptId() {return msgIdCounter++; }

	void sendRequests(ConnectionHandler& ch) {
		while (!terminate) {
			const short bufsize = 1024;
			char buf[bufsize];
			cin.getline(buf, bufsize);
			string line(buf);
			int len=line.length();
            string request = parseInput(line);

            if (request.length() != 0) 
                if (!ch.sendLine(request)) {
                    cout << "Disconnected. Exiting...\n" << endl;
                    terminate = true;
                }
		}
	}

    string parseInput(string input) {
        vector<string> keywords(0);
        int idx = 0;
        while (idx < input.length()){
            int next = input.find(' ');
            keywords.push_back(input.substr(idx, next));
            idx = ++next;
        }

        string& command = keywords.at(0);

        if (command == "login") {
            Frame request("CONNECT");
            if (keywords.size() != 4 || keywords.at(1).size() - keywords.at(1).find(':') != 4 || keywords.at(1).size() < 4) {
                // log illegal input
            }
            request.addHeader("host", keywords.at(1));
            request.addHeader("login", keywords.at(2));
            request.addHeader("passcode", keywords.at(3));
            return request.toStringRepr();
        }

        if (command == "join") {
            Frame request("SUBSCRIBE");
            if (keywords.size() != 2) {
                // log illegal input
            }
            string destination = "/"+keywords.at(1);
            subscriptions[destination] = generateNewSubId();
            request.addHeader("destination", destination);
            request.addHeader("id", to_string(subscriptions[destination]));
            request.addHeader("receipt-id", to_string(generateNewReceiptId()));
            return request.toStringRepr();
        }

        else if (command == "report") {
            Frame request("SEND");
            if (keywords.size() != 2) {
                // log illegal input
                return "";
            }
            string message = readFile(keywords.at(1));
            names_and_events name_and_events = parseEventsString(message);
            string game_name = name_and_events.team_a_name + "_" + name_and_events.team_b_name;
            request.addHeader("destination", "/"+game_name);
            request.body_ = message;
            return request.toStringRepr();
        }
        else if (command == "exit") {
            Frame request("UNSUBSCRIBE");
            if (keywords.size() != 2) {
                // log illegal input
                return "";
            }
            string destination = "/"+keywords.at(1);
            int subId = subscriptions[destination];
            request.addHeader("id", to_string(subId));
            request.addHeader("receipt-id", to_string(generateNewReceiptId()));
            return request.toStringRepr();
        }
        else if (command == "logout") {
            Frame request("DISCONNECT");
            if (keywords.size() != 1) {
                // log illegal input
                return "";
            }
            terminateReceipt = generateNewReceiptId();
            request.addHeader("receipt-id", to_string(terminateReceipt));
            return request.toStringRepr();
        }
        else if (command == "summary") {
            Frame request("DISCONNECT");
            if (keywords.size() != 1) {
                // log illegal input
                return "";
            }
            // TODO: print and save a file or whatever
            return "";
        }
        return "";
    }

	void displayMessages(ConnectionHandler& ch) {
		while (!terminate) {
            string responseString;
            if(!ch.getLine(responseString)){
                //maybe there is a better solution
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                terminate = true;
            }
            Frame response = parseFrame(responseString); 
            string command = response.command_;
            if (command == "ERROR") {

            }
            else if (command == "RECEIPT") {

            }
            else if (command == "MESSAGE") {

            }
            
            //handle serverResponse

            // if (receipt_id == terminateReceipt) terminate = true;
		}
	}


};