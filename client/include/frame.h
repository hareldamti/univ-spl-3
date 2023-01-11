#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

class Frame
{
    public:
        string command_;
        map<string, string> headers_;
        string body_;
        
        Frame(string command, map<string, string> headers, string body);
        Frame(string command);
        
        void addHeader(string key, string value);
        string getHeader(string key);
        string toStringRepr();
};
Frame parseFrame(string serverResponse);