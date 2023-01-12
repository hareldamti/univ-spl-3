#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;
using namespace std;

Event::Event(string team_a_name, string team_b_name, string name, int time,
             map<string, string> game_updates, map<string, string> team_a_updates,
             map<string, string> team_b_updates, string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::~Event()
{
}

const string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const map<string, string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const map<string, string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const map<string, string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const string &Event::get_discription() const
{
    return this->description;
}

Event::Event(const string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description(""){}

string readFile(string path) {
    ifstream inFile;
    inFile.open(path); //open the input file

    stringstream strStream;
    strStream << inFile.rdbuf(); //read the file
    inFile.close();
    return strStream.str(); //str holds the content of the file
}

void writeFile(string json, string path) {
    ofstream outFile;
    try {
        outFile.open(path);
        outFile << json;
        outFile.close();
    }
    catch (...) { cout << "Error while trying to save file" << endl; }
}

names_and_events parseEventsJson(string str) {
    stringstream s(str);
    json data = json::parse(s);
    string team_a_name = data["team a"];
    string team_b_name = data["team b"];
    // run over all the events and convert them to Event objects
    vector<Event> events;
    for (auto &event : data["events"])
    {
        string name = event["event name"];
        int time = event["time"];
        string description = event["description"];
        map<string, string> game_updates;
        map<string, string> team_a_updates;
        map<string, string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}

string formatEventMessage(Event& event, string username) {
    string result = "";
    result += "user: "+username+"\n";
    result += "team a: "+event.get_team_a_name()+"\n";
    result += "team b: "+event.get_team_b_name()+"\n";
    result += "event name: "+event.get_name()+"\n";
    result += "time: "+to_string(event.get_time())+"\n";
    result += "general game updates:\n";
    for (auto& pair : event.get_game_updates()) {
        result += "\t"+ pair.first +": "+ pair.second +"\n";
    }
    result += "team a updates:\n";
    for (auto& pair : event.get_team_a_updates()) {
        result += "\t"+ pair.first +": "+ pair.second +"\n";
    }
    result += "team b updates:\n";
    for (auto& pair : event.get_team_b_updates()) {
        result += "\t"+ pair.first +": "+ pair.second +"\n";
    }
    result += "description:\n";
    result += event.get_discription();
    return result;
}

pair<string, Event> parseEventMessage(string message) {
    vector<string> lines;
    unsigned short idx = 0;
    while (idx < message.length()) {
        int next = message.find('\n', idx);
        if (next == -1) next = message.length();
        lines.push_back(message.substr(idx, next - idx));
        idx = next + 1;
    }

    unsigned short i = 1;
    string user = lines[i].substr(lines[i].find(':')+1,lines[i].length()); i++;
    string team_a_name = lines[i].substr(lines[i].find(':')+1,lines[i].length()); i++;
    string team_b_name = lines[i].substr(lines[i].find(':')+1,lines[i].length()); i++;
    string name = lines[i].substr(lines[i].find(':')+1,lines[i].length()); i++;
    int time = stoi(lines[i].substr(lines[i].find(':')+1,lines[i].length()));i += 2;
    
    //general game updates
    map<string, string> game_updates;
    while (lines[i] != "team a updates:") {
        int seperator = lines[i].find(':');
        game_updates[lines[i].substr(1, seperator - 1)] = lines[i].substr(seperator + 2, lines[i].length());
        i++;
    }
    i ++;
    map<string, string> team_a_updates;
    while (lines[i] != "team b updates:") {
        int seperator = lines[i].find(':');
        team_a_updates[lines[i].substr(1, seperator - 1)] = lines[i].substr(seperator + 2, lines[i].length());
        i++;
    }
    i ++;
    map<string, string> team_b_updates;
    while (lines[i] != "description:") {
        int seperator = lines[i].find(':');
        team_b_updates[lines[i].substr(1, seperator - 1)] = lines[i].substr(seperator + 2, lines[i].length());
        i++;
    }
    i ++;
    string description = "";
    while (i < lines.size()){ 
        description += lines[i] + ((i == lines.size()-1)?"":"\n");
        i++;
        }

    return {user, Event(team_a_name, team_b_name, name, time,
             game_updates, team_a_updates, team_b_updates, description)};
}

string createSummaryString(map<string, map<string, vector<Event>>>& totalEvents,
    string game_name, string username) {
    string result;

    if (totalEvents.find(game_name) == totalEvents.end() ||
        totalEvents[game_name].find(username) == totalEvents[game_name].end())
        return username + "'s updates for " + game_name + " not found";
    
    // Select the desired event list and sort them by time & halftime
    vector<Event>& events = totalEvents[game_name][username];
    sort(events.begin(), events.end(), [](Event const &a, Event const &b) { 
        return (b.get_game_updates().at("before halftime") == "true" && a.get_game_updates().at("before halftime") == "false") ||
        (b.get_game_updates().at("before halftime") == a.get_game_updates().at("before halftime") &&
        b.get_time() > a.get_time());
    });
    Event& last = events.at(events.size()-1);
    result += last.get_team_a_name() + " vs " + last.get_team_b_name() + "\n";
    
    map<string, string> game_updates, team_a_updates, team_b_updates;
    for (auto& event : events) {
        for (auto& pair : event.get_game_updates())
            game_updates[pair.first] = pair.second;
        for (auto& pair : event.get_team_a_updates())
            team_a_updates[pair.first] = pair.second;
        for (auto& pair : event.get_team_b_updates())
            team_b_updates[pair.first] = pair.second;
    }

    result += "Game stats:\n";
    for (auto& pair : game_updates) {
        result += "\t"+ pair.first +": "+ pair.second +"\n";
    }
    result += last.get_team_a_name()+" stats:\n";
    for (auto& pair : team_a_updates) {
        result += "\t"+ pair.first +": "+ pair.second +"\n";
    }
    result += last.get_team_b_name()+" stats:\n";
    for (auto& pair : team_b_updates) {
        result += "\t"+ pair.first +": "+ pair.second +"\n";
    }
    result += "Game event reports:\n";
    for (Event& event : events) {
        result += event.get_time() + " - " + event.get_name() + ":\n\n";
        result += event.get_discription() + "\n\n\n";
    }
    return result;
}