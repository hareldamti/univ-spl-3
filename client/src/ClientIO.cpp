#include "../include/ClientIO.h"
#include "../include/ConnectionHandler.h"
#include "../include/frame.h"
#include "../include/event.h"

#include <iostream>
#include <map>
#include <mutex>
using namespace std;


class ClientIO {
    // TODO: mutex lock totalEvents, state
    
    ConnectionHandler* connectionHandler;

    int nextStateReceipt;
    ClientState state;
    
    string username;
    map<string, int> subscriptions;
    int subIdCounter, msgIdCounter;
    map<string, map<string, vector<Event>>> totalEvents;

    ClientIO(): connectionHandler(nullptr), nextStateReceipt(0), state(AwaitingLogin),
            subIdCounter(1), msgIdCounter(1){}
    
    int generateNewSubId() {return subIdCounter++; }
    int generateNewReceiptId() {return msgIdCounter++; }

    bool startConnection() {
        while (state == ClientState::AwaitingLogin) {
            // read user's line
            const short bufsize = 1024;
            char buf[bufsize];
            cin.getline(buf, bufsize);
            string input(buf);

            // split to keywords
            vector<string> keywords(0);
            int idx = 0;
            while (idx < input.length()){
                int next = input.find(' ', idx);
                if (next == -1) next = input.length();
                keywords.push_back(input.substr(idx, next - idx));
                idx = ++next;
            }
            string command(keywords.at(0));

            //try to connect & login
            if (command == "login") {
                Frame request("CONNECT");
                if (keywords.size() != 4 || keywords.at(1).find(':') == -1) {
                    cout << "Unsent - login format: login {host:port} {username} {password}" << endl;
                }
                else {
                    username = keywords.at(2);

                    int seperator = keywords.at(1).find(':');
                    string host = keywords.at(1).substr(0,seperator);
                    int port = stoi(keywords.at(1).substr(seperator + 1, 4));

                    connectionHandler = new ConnectionHandler(host, port);
                    if (!connectionHandler->connect()) {
                        cerr << "Cannot connect to " << host << ":" << port << endl;
                        return false;
                    }
                    cout << "Connected to the server. logging in..." << endl;

                    nextStateReceipt = generateNewReceiptId();
                    request.addHeader("host", keywords.at(1));
                    request.addHeader("login", keywords.at(2));
                    request.addHeader("passcode", keywords.at(3));
                    request.addHeader("receipt-id", to_string(nextStateReceipt));
                    sendStompFrame(request);
                    state = ClientState::AwaitingConnected;
                }
            }

            else {
                cout << "Unsent - send a login command first" << endl; return;
            }
        }

    }

	void sendRequests() {
		while (state != ClientState::Disconnected && state != ClientState::AwatingDisconnect) {
			const short bufsize = 1024;
			char buf[bufsize];
			cin.getline(buf, bufsize);
			string line(buf);
			int len=line.length();
            processInput(line);
		}
	}

    void processInput(string input) {

        // split input to keywords
        vector<string> keywords(0);
        int idx = 0;
        while (idx < input.length()){
            int next = input.find(' ', idx);
            if (next == -1) next = input.length();
            keywords.push_back(input.substr(idx, next - idx));
            idx = ++next;
        }
        string command = keywords.at(0);

        if (command == "login") {
            cout << "You are already logged in" << endl; return;
        }

        else if (command == "join") {
            Frame request("SUBSCRIBE");
            if (keywords.size() != 2) {
                cout << "Unsent - join format: join {game name} (game name = {team a}_{team_b})" << endl; return;
            }
            string destination = "/"+keywords.at(1);
            subscriptions[destination] = generateNewSubId();
            request.addHeader("destination", destination);
            request.addHeader("id", to_string(subscriptions[destination]));
            request.addHeader("receipt-id", to_string(generateNewReceiptId()));
            sendStompFrame(request);
        }

        else if (command == "report") {
            if (keywords.size() != 2) {
                cout << "Unsent - report format: report {path.json})" << endl; return;
            }

            string eventsJson;
            try { eventsJson = readFile(keywords.at(1)); }
            catch (...) {cout << "Unsent - can't open file (did you type a correct file name? is it open?)" << endl; return;}
            
            names_and_events names_and_events;
            try { names_and_events = parseEventsJson(eventsJson); }
            catch (...) {cout << "Unsent - can't parse file (make sure the json is formatted correctly))" << endl; return;}
            
            string game_name = names_and_events.team_a_name + "_" + names_and_events.team_b_name;
            string destination = "/"+game_name;

            if (totalEvents.find(destination) == totalEvents.end())
            { cout << "Unsent - You are not subscribed to " << destination << endl; return; }

            for (auto& event : names_and_events.events) {
                Frame eventFrame("SEND");
                eventFrame.addHeader("destination", destination);
                eventFrame.body_ = formatEventMessage(event, username);
                sendStompFrame(eventFrame);
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
            sendStompFrame(request);
        }

        else if (command == "logout") {
            Frame request("DISCONNECT");
            if (keywords.size() != 1) {
                cout << "Unsent - logout format: logout" << endl; return;
            }
            terminateReceipt = generateNewReceiptId();
            request.addHeader("receipt-id", to_string(terminateReceipt));
            sendStompFrame(request);
        }
        
        else if (command == "summary") {
            if (keywords.size() != 4) {
                cout << "summary format: summary {game_name} {user} {path}" << endl; return;
            }

            string game_name = keywords.at(1);
            string user = keywords.at(2);
            string path = keywords.at(3);

            string output = createSummaryString(totalEvents, "/"+game_name, user);
            writeFile(output, path);
        }
    }

    void sendStompFrame(Frame& frame) {
        string msg = frame.toStringRepr();
        if (!connectionHandler->sendLine(msg)) {
            cout << "Disconnected. Exiting...\n" << endl;
            terminate = true;
        }
    }

	void processMessages() {
		while (state != ClientState::Disconnected) {
            
            string responseString;
            if(!connectionHandler->getLine(responseString)){
                cout << "Disconnected. Exiting...\n" << endl;
                state = ClientState::Disconnected;
            }

            Frame response = parseFrame(responseString);
            string command = response.command_;
            
            if (command == "CONNECTED") {
                if (state == ClientState::AwatingConnected && stoi(response.getHeader("receipt-id")) == nextStateReceipt) {
                    state = ClientState::Connected;
                }
            }
            
            else if (command == "ERROR") {
                cout << "Received an error message from the server: " << response.getHeader("message")  << "\tFull error message: \n\n";
                cout << response.body_;
                state = ClientState::Disconnected;
            }
            
            else if (command == "RECEIPT") {
                if (state == ClientState::AwatingDisconnected && stoi(response.getHeader("receipt-id")) == nextStateReceipt) {
                    cout << "Disconnected successfully " << endl;
                    state = ClientState::Disconnected;
                }
            }
            else if (command == "MESSAGE") {
                pair<string, Event> receivedEvent = parseEventMessage(response.body_);
                if (receivedEvent.first != username){
                    totalEvents[reponse.getHeader("destination")][receivedEvent.first].push_back(receivedEvent.second);
                    cout << "--Update received-- at "+reponse.getHeader("destination") << "\n\n";
                    cout << response.body_ << endl;
                }
            }
		}
	}

    ~ClientIO() {
        delete connectionHandler;
    }
};

