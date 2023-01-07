
#include "../include/frame.h"

using namespace std;
Frame::Frame(string command, std::map<string, string> headers, string body) :
    command_(command),
    headers_(headers),
    body_(body) {}

Frame::Frame(string command) : command_(command), body_(""){}

void Frame::addHeader(string key, string value) {
    headers_[key] = value;
}

string Frame::toStringRepr() {
    string str = command_+"\n";
    for (auto& header : headers_) {
        str += header.first + ":" + header.second + "\n";
    }
    str += "\n" + body_ + "\u0000";
}


string parseResponse(string input)
{
    return string();
}

