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
        string event_;
        
        Frame(string command, map<string, string> headers, string event);
        Frame(string command);
        ~Frame();

        void addHeader(string key, string value);
};

string parseInput(string input);
string parseResponse(string input);