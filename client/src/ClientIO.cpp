#include "../include/ClientIO.h"
#include <iostream>
#include <map>
#include <mutex>
#include "../include/frame.h"
using namespace std;


ClientIO::ClientIO(): connectionHandler(nullptr), nextStateReceipt(0), state(AwaitingLogin),
        username(""), subIdCounter(1), msgIdCounter(1){}

ClientIO::ClientIO(const ClientIO& other):
    nextStateReceipt(other.nextStateReceipt),
    state(other.state),
    username(other.username),
    subIdCounter(other.subIdCounter),
    msgIdCounter(other.msgIdCounter) {
        connectionHandler = new ConnectionHandler(other.connectionHandler->host_, other.connectionHandler->port_);
    }

ClientIO::ClientIO(const ClientIO&& other):
    connectionHandler(other.connectionHandler),
    nextStateReceipt(other.nextStateReceipt),
    state(other.state),
    username(other.username),
    subIdCounter(other.subIdCounter),
    msgIdCounter(other.msgIdCounter) {}

ClientIO& ClientIO::operator=(const ClientIO& other) {
    if (this != &other) {
        delete connectionHandler;
        connectionHandler = new ConnectionHandler(other.connectionHandler->host_, other.connectionHandler->port_);
        nextStateReceipt = other.nextStateReceipt;
        state = other.state;
        username = other.username;
        subIdCounter = other.subIdCounter;
        msgIdCounter = other.msgIdCounter;
    }
    return *this;
}

ClientIO& ClientIO::operator=(const ClientIO&& other) {
    if (this != &other) {
        delete connectionHandler;
        connectionHandler = other.connectionHandler;
        nextStateReceipt = other.nextStateReceipt;
        state = other.state;
        username = other.username;
        subIdCounter = other.subIdCounter;
        msgIdCounter = other.msgIdCounter;
    }
    return *this;
}

int ClientIO::generateNewSubId() {return subIdCounter++; }

int ClientIO::generateNewReceiptId() {return msgIdCounter++; }

void ClientIO::setState(ClientState newState) {
    lock_guard<mutex> sync(stateLock);
    state = newState;
    sync.~lock_guard();
}

bool ClientIO::compareState(ClientState otherState) {
    lock_guard<mutex> sync(stateLock);
    return state == otherState;
}

bool ClientIO::startConnection() {
    while (state == ClientState::AwaitingLogin) {
        // read user's line
        const short bufsize = 1024;
        char buf[bufsize];
        cin.getline(buf, bufsize);
        string input(buf);

        // split to keywords
        vector<string> keywords(0);
        unsigned short idx = 0;
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
                request.addHeader("accept-version", "1.2");
                request.addHeader("host", host);
                request.addHeader("login", keywords.at(2));
                request.addHeader("passcode", keywords.at(3));
                request.addHeader("receipt-id", to_string(nextStateReceipt));
                sendStompFrame(request);
                state = ClientState::AwaitingConnected;
            }
        }
        else {
            cout << "Unsent - send a login command first" << endl;
        }
    }
    return true;
}

void ClientIO::sendRequests() {
    while (!compareState(ClientState::Disconnected) && !compareState(ClientState::AwaitingDisconnected)) {
        const short bufsize = 1024;
        char buf[bufsize];
        cin.getline(buf, bufsize);
        string line(buf);
        processInput(line);
    }
}

void ClientIO::processInput(string input) {

    // split input to keywords
    vector<string> keywords(0);
    unsigned short idx = 0;
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

        lock_guard<mutex> sync(eventsLock);
        if (totalEvents.find(destination) == totalEvents.end())
        { cout << "Unsent - You are not subscribed to " << destination << endl; return; }
        sync.~lock_guard();
        
        for (auto& event : names_and_events.events) {
            Frame eventFrame("SEND");
            eventFrame.addHeader("destination", destination);
            eventFrame.body_ = formatEventMessage(event, username);
            sendStompFrame(eventFrame);
            
            lock_guard<mutex> sync(eventsLock);
            totalEvents[destination][username].push_back(event);
            sync.~lock_guard();
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
        nextStateReceipt = generateNewReceiptId();
        setState(ClientState::AwaitingDisconnected);
        request.addHeader("receipt-id", to_string(nextStateReceipt));
        sendStompFrame(request);
    }
    
    else if (command == "summary") {
        if (keywords.size() != 4) {
            cout << "summary format: summary {game_name} {user} {path}" << endl; return;
        }

        string game_name = keywords.at(1);
        string user = keywords.at(2);
        string path = keywords.at(3);

        string output;
        lock_guard<mutex> sync(eventsLock);
        output = createSummaryString(totalEvents, "/"+game_name, user);
        sync.~lock_guard();
        
        writeFile(output, path);
    }
}

void ClientIO::sendStompFrame(Frame& frame) {
    string msg = frame.toStringRepr();
    if (!connectionHandler->sendLine(msg)) {
        cout << "Disconnected. Exiting...\n" << endl;
        setState(ClientState::Disconnected);
    }
}

void ClientIO::processMessages() {
    while (!compareState(ClientState::Disconnected)) {
        string responseString;
        if(!connectionHandler->getLine(responseString)){
            cout << "Disconnected. Exiting...\n" << endl;
            setState(ClientState::Disconnected);
        }
        try {
            Frame response = parseFrame(responseString);
            string command = response.command_;
            if (command == "CONNECTED") {
                if (compareState(ClientState::AwaitingConnected) && stoi(response.getHeader("receipt-id")) == nextStateReceipt) {
                    setState(ClientState::Connected);
                }
            }
            
            else if (command == "ERROR") {
                cout << "Received an error message from the server: " << response.getHeader("message")  << "\nFull error message: \n\n";
                cout << response.body_;
                setState(ClientState::Disconnected);
            }
            
            else if (command == "RECEIPT") {
                if (compareState(ClientState::AwaitingDisconnected) && stoi(response.getHeader("receipt-id")) == nextStateReceipt) {
                    cout << "Disconnected successfully " << endl;
                    setState(ClientState::Disconnected);
                }
            }
            
            else if (command == "MESSAGE") {
                pair<string, Event> receivedEvent = parseEventMessage(response.body_);
                if (receivedEvent.first != username){
                    lock_guard<mutex> sync(eventsLock);
                    totalEvents[response.getHeader("destination")][receivedEvent.first].push_back(receivedEvent.second);
                    sync.~lock_guard();
                    cout << "--Update received-- at "+response.getHeader("destination") << "\n\n";
                    cout << response.body_ << endl;
                }
            }
        }
        catch (exception& e) {cout << "Error in response processing: " << e.what() << endl; }
    }
}

ClientIO::~ClientIO() {
    delete connectionHandler;
}


