
#include "../include/frame.h"
#include <vector>


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

Frame parseFrame(string serverResponse){
    vector<string> splittedResponse;
    int idx = 0;
    while (idx < serverResponse.length()) {
        int next = serverResponse.find('\n', idx);
        if (next == -1) next = serverResponse.length();
        splittedResponse.push_back(serverResponse.substr(idx, next - idx));
        if (idx == next) {
            if (next + 1 < serverResponse.length())
                splittedResponse.push_back(serverResponse.substr(next + 1 , serverResponse.length()));
            break;
        }
        idx = next + 1;
    }

    //creates a frame with the command from the server response
    Frame parsedFrame = Frame(splittedResponse.at(0));
    int lineIdx = 1;

    //parses the headers from server to the frame's map
    while(lineIdx < splittedResponse.size() && !splittedResponse.at(lineIdx).empty()){
        vector<string> headerPair;
        string header = splittedResponse.at(lineIdx);
        int seperator = header.find(':');
        parsedFrame.addHeader(header.substr(0, seperator), header.substr(seperator+1,header.length()));
        lineIdx++;
    }
    lineIdx++;
    //if there is a message body, adds it to the frame
    if (lineIdx < splittedResponse.size()) parsedFrame.body_ = splittedResponse.at(lineIdx);
    return parsedFrame;
}
