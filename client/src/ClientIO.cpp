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
                cout << "Unsent - login format: login {host:port} {username} {password}" << endl;
                return "";
            }
            request.addHeader("host", keywords.at(1));
            request.addHeader("login", keywords.at(2));
            request.addHeader("passcode", keywords.at(3));
            return request.toStringRepr();
        }

        if (command == "join") {
            Frame request("SUBSCRIBE");
            if (keywords.size() != 2) {
                cout << "Unsent - join format: join {game name} (game name = {team a}_{team_b})" << endl;
                return "";
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
                cout << "Unsent - send format: send {path.json})" << endl;
                return "";
            }
            
            try { string message = readFile(keywords.at(1)); }
            catch (...) {cout << "Unsent - can't open file (did you type a correct file name? is it open?)" << endl; return "";}
            
            try { names_and_events name_and_events = parseEventsString(message); }
            catch (...) {cout << "Unsent - can't parse file (make sure the json is formatted correctly))" << endl; return "";}
            
            string game_name = name_and_events.team_a_name + "_" + name_and_events.team_b_name;
            request.addHeader("destination", "/"+game_name);
            request.addHeader("id", );
            request.body_ = message;
            return request.toStringRepr();
        }

        else if (command == "exit") {
            Frame request("UNSUBSCRIBE");
            if (keywords.size() != 2) {
                cout << "Unsent - exit format: exit {game_name} (game name = {team a}_{team_b})" << endl;
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
                cout << "Unsent - logout format: logout" << endl;
                return "";
            }
            terminateReceipt = generateNewReceiptId();
            request.addHeader("receipt-id", to_string(terminateReceipt));
            return request.toStringRepr();
        }
        
        else if (command == "summary") {
            if (keywords.size() != 2) {
                cout << "summary format: summary {path.json}" << endl;
                return "";
            }
            // TODO: save the summary to filename path
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
                terminate = true;
            }
            else if (command == "RECEIPT") {
                //should we log it for debug purposes?
                if(stoi(response.headers_["reciept-id"]) == terminateReceipt) terminate = true;
            }
            else if (command == "MESSAGE") {
                names_and_events name_and_events = parseEventsString(response.body_);
                //save to a file and display
            }
		}
	}
};