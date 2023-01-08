
#include "../include/frame.h"
#include <vector>
//this is for the split function
#include <boost/algorithm/string.hpp>
#include <bits/stdc++.h>


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
    boost::split(splittedResponse, serverResponse, boost::is_any_of("\n"));
    int lineIdx = 0;

    //creates a frame with the command from the server response
    Frame parsedFrame = Frame(splittedResponse.at(lineIdx));
    lineIdx++;

    //parses the headers from server to the frame's map
    while(lineIdx < splittedResponse.size() && !splittedResponse.at(lineIdx).empty()){
        vector<string> headerPair;
        boost::split(headerPair, splittedResponse.at(lineIdx), boost::is_any_of(":"));
        parsedFrame.addHeader(headerPair.at(0), headerPair.at(1));
        lineIdx++;
    }
    lineIdx++;
    
    //if there is a message body, adds it to the frame
    for (; lineIdx < splittedResponse.size(); lineIdx++)
        parsedFrame.body_ += splittedResponse.at(lineIdx) + "\n";

    return parsedFrame;
}






string parseResponse(string input)
{
    return string();
}

