#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_discription() const
{
    return this->description;
}

Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description(""){}

std::string readFile(std::string path) {
    std::ifstream inFile;
    inFile.open(path); //open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf(); //read the file
    inFile.close();
    return strStream.str(); //str holds the content of the file
}

void writeFile(std::string json, std::string path) {
    std::ofstream outFile;
    try {
        outFile.open(path);
        outFile << json;
        outFile.close();
    }
    catch (...) { std::cout << "Error while trying to save file" << std::endl; }
}

names_and_events parseEventsJson(std::string str) {
    std::stringstream s(str);
    json data = json::parse(s);
    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];
    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
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

std::string formatEventMessage(Event& event, string username) {
    string result = "";
    result += "user: "+username+"\n";
    result += "team a: "+event.get_team_a_name()+"\n";
    result += "team a: "+event.get_team_a_name()+"\n";
    result += "event name: "+event.get_name()+"\n";
    result += "time: "+std::to_string(event.get_time())+"\n";
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

std::string createSummaryString(std::map<string, std::map<string, std::vector<Event>>>& totalEvents,
    std::string game_name, std::string username) {
    std::string result;

    if (totalEvents.find(game_name) == totalEvents.end() ||
        totalEvents[game_name].find(username) == totalEvents[game_name].end())
        return username + "'s updates for " + game_name + " not found";
    
    // Select the desired event list and sort them by time & halftime
    std::vector<Event>& events = totalEvents[game_name][username];
    std::sort(events.begin(), events.end(), [](Event const &a, Event const &b) { 
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

{
    std::ifstream f(json_path);
    json data = json::parse(f);
    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
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