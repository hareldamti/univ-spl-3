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

    bool connected;
    string username;
    map<string, int> subscriptions;
    int subIdCounter, msgIdCounter;
    
    map<string, map<string, vector<Event>>> totalEvents;

    ClientIO(ConnectionHandler& ch): terminate(false), terminateReceipt(0), connected(false),
            subIdCounter(1), msgIdCounter(1){}
    
    int generateNewSubId() {return subIdCounter++; }
    int generateNewReceiptId() {return msgIdCounter++; }

	void sendRequests(ConnectionHandler& ch) {
		while (!terminate) {
			const short bufsize = 1024;
			char buf[bufsize];
			cin.getline(buf, bufsize);
			string line(buf);
			int len=line.length();
            processInput(line, ch);
		}
	}

    void processInput(string input, ConnectionHandler& ch) {
        vector<string> keywords(0);
        int idx = 0;
        while (idx < input.length()){
            int next = input.find(' ', idx);
            if (next == -1) next = input.length();
            keywords.push_back(input.substr(idx, next - idx));
            idx = ++next;
        }
        string& command = keywords.at(0);

        if (command == "login") {
            Frame request("CONNECT");
            if (keywords.size() != 4 || keywords.at(1).find(':') == -1) {
                cout << "Unsent - login format: login {host:port} {username} {password}" << endl; return;
            }
            username = keywords.at(2);
            request.addHeader("host", keywords.at(1));
            request.addHeader("login", keywords.at(2));
            request.addHeader("passcode", keywords.at(3));
            sendStompFrame(request, ch);
        }

        if (command == "join") {
            Frame request("SUBSCRIBE");
            if (keywords.size() != 2) {
                cout << "Unsent - join format: join {game name} (game name = {team a}_{team_b})" << endl; return;
            }
            string destination = "/"+keywords.at(1);
            subscriptions[destination] = generateNewSubId();
            request.addHeader("destination", destination);
            request.addHeader("id", to_string(subscriptions[destination]));
            request.addHeader("receipt-id", to_string(generateNewReceiptId()));
            sendStompFrame(request, ch);
        }

        else if (command == "report") {
            if (keywords.size() != 2) {
                cout << "Unsent - send format: send {path.json})" << endl; return;
            }

            string eventsJson;
            try { eventsJson = readFile(keywords.at(1)); }
            catch (...) {cout << "Unsent - can't open file (did you type a correct file name? is it open?)" << endl; return;}
            
            names_and_events names_and_events;
            try { names_and_events = parseEventsJson(eventsJson); }
            catch (...) {cout << "Unsent - can't parse file (make sure the json is formatted correctly))" << endl; return;}
            
            string game_name = names_and_events.team_a_name + "_" + names_and_events.team_b_name;
            string destination = "/"+game_name;
            for (auto& event : names_and_events.events) {
                Frame eventFrame("SEND");
                eventFrame.addHeader("destination", destination);
                eventFrame.body_ = formatEventMessage(event, username);
                sendStompFrame(eventFrame, ch);
                totalEvents[destination][username].push_back(event);
            }
        }

        else if (command == "exit") {
            Frame request("UNSUBSCRIBE");
            if (keywords.size() != 2) {
                cout << "Unsent - exit format: exit {game_name} (game name = {team a}_{team_b})" << endl; return;
            }
            string destination = "/"+keywords.at(1);
            int subId = subscriptions[destination];
            subscriptions.erase(destination);
            request.addHeader("id", to_string(subId));
            request.addHeader("receipt-id", to_string(generateNewReceiptId()));
            sendStompFrame(request, ch);
        }

        else if (command == "logout") {
            Frame request("DISCONNECT");
            if (keywords.size() != 1) {
                cout << "Unsent - logout format: logout" << endl; return;
            }
            terminateReceipt = generateNewReceiptId();
            request.addHeader("receipt-id", to_string(terminateReceipt));
            sendStompFrame(request, ch);
        }
        
        else if (command == "summary") {
            if (keywords.size() != 2) {
                cout << "summary format: summary {path.json}" << endl; return;
            }
            // TODO: save the summary to filename path
            
        }
    }

    void sendStompFrame(Frame& frame, ConnectionHandler& ch) {
        string msg = frame.toStringRepr();
        if (!ch.sendLine(msg)) {
            cout << "Disconnected. Exiting...\n" << endl;
            terminate = true;
        }
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
                connected = false;
            }
            else if (command == "CONNECTED") { connected = true; }
            else if (command == "RECEIPT") {
                if(stoi(response.headers_["reciept-id"]) == terminateReceipt)
                    { terminate = true; connected = false; }
            }
            else if (command == "MESSAGE") {
                names_and_events name_and_events = parseEventsString(response.body_);
                //save to a file and display
            }
		}
	}
};