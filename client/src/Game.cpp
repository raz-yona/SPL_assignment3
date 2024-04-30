#include "../include/Game.h"

Game::Game(std::string teams_names) : team_a_name(""), team_b_name(""), events_by_user(std::map<std::string, std::vector<std::string>>()), gameUpdates(std::map<std::string, std::map<std::string, string>>()) {
    int pos = teams_names.find("_");
    team_a_name = teams_names.substr(0, pos);
    team_b_name = teams_names.substr(pos + 1, teams_names.length());
}


void Game::add_event(std::string user, std::string event) {
    events_by_user[user].push_back(event);
}



std::vector<std::string> Game::get_events_by_user(std::string user) {
    return events_by_user[user];
}

std::string Game::get_teams_names(){
    return team_a_name + "_" + team_b_name;
}

void Game::updateGame(std::map<std::string, std::map<std::string, string>> newGameUpdates){
    gameUpdates = newGameUpdates;
}

std::map<std::string, std::map<std::string, string>> Game::getGameUpdates(){
    return gameUpdates;
}