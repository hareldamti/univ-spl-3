
#include "../include/frame.h"

using namespace std;
Frame::Frame(string command, std::map<string, string> headers, string event) :
    command_(command),
    headers_(headers),
    event_(event) {}

Frame::Frame(string command) : command_(command) {}

void Frame::addHeader(string key, string value) {
    headers_[key] = value;
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
    }
    if (command == "join") {

    }
    if (command == "report") {

    }
    if (command == "exit") {

    }
    if (command == "logout") {

    }
    if (command == "summary") {

    }
    return string();
}

string parseResponse(string input)
{
    return string();
}

