#include <iostream>
#include <fstream>
#include "StompProtocol.h"
using namespace std;

StompProtocol::StompProtocol(ConnectionHandler* connectionHandler, std::string username) : connectionHandler(connectionHandler), username(username), receiptIdGenerator(0), subscriptionIdGenerator(0), connected(false), games(std::map<std::string, Game*>()), gamesByUser(std::map<std::string, std::vector<std::string>>()), receiptIdToResponseData(std::map<std::string, vector<string>>()), subscriptionIdByUser(std::map<std::string, std::string>()), userBySubscriptionId(std::map<std::string, std::string>()) {
}

StompProtocol::~StompProtocol() {
  if(connectionHandler){
    delete connectionHandler;
  }
  if(games.size() > 0){
    for(auto it = games.begin(); it != games.end(); it++){
      delete it->second;
    }
  }
}

//copy constructor
StompProtocol::StompProtocol(const StompProtocol& other) : connectionHandler(other.connectionHandler), username(other.username), receiptIdGenerator(other.receiptIdGenerator), subscriptionIdGenerator(other.subscriptionIdGenerator), connected(other.connected), games(other.games), gamesByUser(other.gamesByUser), receiptIdToResponseData(other.receiptIdToResponseData), subscriptionIdByUser(other.subscriptionIdByUser), userBySubscriptionId(other.userBySubscriptionId){
}

//copy assignment operator
StompProtocol& StompProtocol::operator=(const StompProtocol& other){
  if(this != &other){
    connectionHandler = other.connectionHandler;
    username = other.username;
    receiptIdGenerator = other.receiptIdGenerator;
    subscriptionIdGenerator = other.subscriptionIdGenerator;
    connected = other.connected;
    games = other.games;
    gamesByUser = other.gamesByUser;
    receiptIdToResponseData = other.receiptIdToResponseData;
    subscriptionIdByUser = other.subscriptionIdByUser;
    userBySubscriptionId = other.userBySubscriptionId;
  }
  return *this;
}

void StompProtocol::processFromUser(std::string line) {
  std::vector<std::string> words = split(line, ' ');
  std::string command = words[0];
  if(command == "login"){
    if(connected){
        std::cout << "The client is already logged in, log out before trying again" << std::endl;
        return;
    }
  }
  else if(username == ""){
    std::cout << "You must login first" << endl;
    return;
  }
  else if(command == "join"){
    std::string frame = "SUBSCRIBE\ndestination:" + words[1] + "\nid:" + std::to_string(subscriptionIdGenerator) + "\nreceipt:" + std::to_string(++receiptIdGenerator) + "\n\n\0";
    //insert the info to the receiptIdToResponseData map
    vector<string> response;
    response.push_back("join");
    response.push_back(words[1]); // game name
    receiptIdToResponseData[std::to_string(receiptIdGenerator)] = response;
    if(!connectionHandler->sendLine(frame)){
        receiptIdToResponseData.erase(std::to_string(receiptIdGenerator--));
        std::cout << "Error: could not send frame to server\n" << std::endl;
    }
  }
  else if (command == "exit"){
    std::string frame = "UNSUBSCRIBE\nid:" + subscriptionIdByUser[username] + "\nreceipt:" + std::to_string(++receiptIdGenerator) + "\n\n\0";
    //insert the info to the receiptIdToResponseData map
    vector<string> response;
    response.push_back("exit");
    response.push_back(words[1]);
    receiptIdToResponseData[std::to_string(receiptIdGenerator)] = response;
    //send the frame and erase from map if it isn't sent
    if(!connectionHandler->sendLine(frame)){
        receiptIdToResponseData.erase(std::to_string(receiptIdGenerator--));
        std::cout << "Error: could not send frame to server\n" << std::endl;
    }
  }
  else if (command == "report"){
    names_and_events names_events = parseEventsFile(words[1]);
    string teamA = names_events.team_a_name;
    string teamB = names_events.team_b_name;
    vector<Event> events = names_events.events;
    for(int i = 0; i < (int)events.size(); i++){
      //Send a message to the server
      std::string frame = "SEND\ndestination:" + teamA + "_" + teamB + "\n\n" + "user: " + username + "\nteam a:" + teamA + "\nteam b:" + teamB + "\n" + events[i].to_string() + "\n\0";
      if(!connectionHandler->sendLine(frame)){
        std::cout << "Error: could not send frame to server\n" << std::endl;
      }
    }
  }
  else if (command == "logout"){
    string frame = "DISCONNECT\nreceipt:" + std::to_string(++receiptIdGenerator) + "\n\n\0";
    vector<string> response;
    response.push_back("logout");
    receiptIdToResponseData[std::to_string(receiptIdGenerator)] = response;
    if(!connectionHandler->sendLine(frame)){
      std::cout << "Error: could not send frame to server\n" << std::endl;
      receiptIdToResponseData.erase(std::to_string(receiptIdGenerator--));
    }
  }
  else if(command == "summary"){
    string teams_names = words[1];
    Game* currentGame = games[teams_names];
    vector<string> teams = split(teams_names, '_');
    string teamA = teams[0];
    string teamB = teams[1];
    string output = teamA + " vs " + teamB + "\nGame stats:\nGeneral stats:\n";
    std::map<std::string, std::map<std::string, std::string>> updates = currentGame->getGameUpdates();
    map<std::string, std::string> generalUpdates = updates["general game updates"];
    map<std::string, std::string> teamAUpdates = updates["team a updates"];
    map<std::string, std::string> teamBUpdates = updates["team b updates"];
    for(auto& key : generalUpdates){
      output += key.first + ": " + key.second + "\n";
    }
    output += teamA + " stats:\n";
    for(auto& key : teamAUpdates){
      output += key.first + ": " + key.second + "\n";
    }
    output += teamB + " stats:\n";
    for(auto& key : teamBUpdates){
      output += key.first + ": " + key.second + "\n";
    }
    std::vector<std::string> events = currentGame->get_events_by_user(username);
    for(int i = 0; i < (int)events.size(); i++){
      output += events[i] + "\n";
    }
    //write the string to the file
    ofstream outputFile;
    outputFile.open(words[3]);
    if(!outputFile.is_open()){
        std::cout << "Error: could not open file\n" << std::endl;
    }
    else{
      outputFile << output;
      outputFile.close();
    }
  }
}

void StompProtocol::processFromServer() {
  std::string line;
  if(!connectionHandler->getLine(line)){
      std::cout << "Could not connect to server" << std::endl;
      return;
  }
  std::string name;
  std::map<std::string, std::string> headers;
  std::string body;
  std::tie(name, headers, body) = parseStompFrame(line);
  if(name != "RECEIPT" && name != "MESSAGE" && name != "ERROR" && name != "CONNECTED") {
      std::cout << "Error: the server sent an unknown frame. Exiting...\n" << std::endl;
  }
  else{
    if(name == "RECEIPT"){
      if(receiptIdToResponseData[headers["receipt-id"]][0] == "join"){
        
        //map[receipt-id][0] is the command,map[receipt-id][1] is the game name
        std::string gameName = receiptIdToResponseData[headers["receipt-id"]][1];
        if (games.find(gameName) == games.end()){
        games[gameName] = new Game(gameName);
        addGameToUser(username, gameName);
        std::cout << "Joined channel " << gameName << endl;
        }
        else if(std::find(gamesByUser[username].begin(), gamesByUser[username].end(), gameName) == gamesByUser[username].end()){
          addGameToUser(username, gameName);
          std::cout << "Joined channel " << gameName << endl;
        }
        else{
          std::cout << "Error: you are already subscribed to this game\n" << std::endl;
        }
      }
      else if(receiptIdToResponseData[headers["receipt-id"]][0] == "exit"){
        //remove the subscrition id from the user
        std::string gameName = receiptIdToResponseData[headers["receipt-id"]][1];
        removeGameFromUser(username, gameName);
        std::cout << "Exited channel " + gameName << endl;
      }
      else if(receiptIdToResponseData[headers["receipt-id"]][0] == "logout"){
        clear();
        try{
          connectionHandler->close();
          std::cout << "Disconnected from server" << endl;
        }
        catch(std::exception& e){
          std::cout << "Error: could not close connection\n" << std::endl;
        }
      }
      // else if(receiptIdToResponseData[headers["receipt-id"]][0] == "login"){
      //   std::string userTemp = receiptIdToResponseData[headers["receipt-id"]][1];
        
      // }
    }
    else if(name == "MESSAGE"){
        //get the user name by the subscription id
        std::string messageUser = userBySubscriptionId[headers["subscription"]];
        //get the game events
        std::map<std::string, std::map<std::string, std::string>> updates = parseMessage(body);
        //update the game
        games[headers["destination"]]->updateGame(updates);
        //add the event as a string to the game
        games[headers["destination"]]->add_event(messageUser, body);
    }
    else if(name == "ERROR"){
        std::cout << headers.at("message") << std::endl; //should be the same with liorrrrr
          setDisconnected();
        try{
          connectionHandler->close();
        }
        catch(std::exception& e){
          std::cout << "Error: could not close connection\n" << std::endl;
        }
    }
  }
}
    
std::tuple<std::string, std::map<std::string, std::string>, std::string> StompProtocol::parseStompFrame(const std::string& frame) {
  std::vector<std::string> lines = split(frame, '\n');
  std::string command = lines[0];
  std::map<std::string, std::string> headers;
  std::string body;
  bool reading_headers = true;
  for (int i = 1; i < (int)lines.size(); i++) {
    const std::string& line = lines[i];
    if (line == "") {
      reading_headers = false;
      continue;
    }
    if (reading_headers) {
      int pos = line.find(':');
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      headers[key] = value;
    } else {
      body += line + "\n";
    }
  }
  return {command, headers, body.substr(0, body.size() - 1)};
}

bool StompProtocol::isConnected() {
  return connected;
}

void StompProtocol::setConnected() {
  connected = true;
}

void StompProtocol::setDisconnected() {
  connected = false;
}

std::string getInputFromUser() {
  const short bufsize = 1024;
  char buf[bufsize];
  std::cin.getline(buf, bufsize); //Reads from the keyboard into buf
  std::string line(buf); //Converts the char array to a string
  return line;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
  std::vector<std::string> words;
  std::string word;
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == delimiter) {
      if (!word.empty()) {
        words.push_back(word);
      }
      word.clear();
    } else {
      word += s[i];
    }
  }
  if (!word.empty()) {
    words.push_back(word);
  }
  return words;
}

void StompProtocol::addGameToUser(std::string user, std::string game){
  gamesByUser[user].push_back(game);
}

void StompProtocol::addSubscriptionId(std::string user){
  std::string subscriptionId = std::to_string(++subscriptionIdGenerator);
  subscriptionIdByUser[user] = subscriptionId;
  userBySubscriptionId[subscriptionId] = user;
}

std::string StompProtocol::getSubscriptionId(std::string user){
  return subscriptionIdByUser[user];
}

void StompProtocol::removeSubscriptionId(std::string id){
  std::string user = userBySubscriptionId[id];
  subscriptionIdByUser.erase(user);
  userBySubscriptionId.erase(id);
}

std::string StompProtocol::getUsername(){
  return username;
}

std::map<std::string, std::map<std::string, std::string>> StompProtocol::parseMessage(std::string message){
  std::map<std::string, std::map<std::string, std::string>> allGameUpdates;
  int generalGameUpdatesPos = message.find("general game updates:") + 21;
  int teamAUpdatesPos = message.find("team a updates:") + 15;
  int teamBUpdatesPos = message.find( " updates:") + 15;
  int descriptionPos = message.find("description:") + 12;
  std::string generalGameUpdates = message.substr(generalGameUpdatesPos, teamAUpdatesPos);
  std::string teamAUpdates = message.substr(teamAUpdatesPos, teamBUpdatesPos);
  std::string teamBUpdates = message.substr(teamBUpdatesPos, descriptionPos);
  std::string eventDescription = message.substr(descriptionPos);
  std::vector<std::string> generalSplitByRow = split(generalGameUpdates, '\n');
  std::vector<std::string> teamASplitByRow = split(teamAUpdates, '\n');
  std::vector<std::string> teamBSplitByRow = split(teamBUpdates, '\n');
  std::vector<std::string> descriptionSplitByRow = split(eventDescription, '\n');
  std::vector<std::string> splitByColumns;
  
  //general game updates
  for(int i = 0; i < (int)generalSplitByRow.size(); i++){
    splitByColumns = split(generalSplitByRow[i],':');
    allGameUpdates["general game updates"][splitByColumns[0]] = splitByColumns[1];
  }
  //team a updates
  for(int i = 0; i < (int)teamASplitByRow.size(); i++){
    splitByColumns = split(teamASplitByRow[i],':');
    allGameUpdates["team a updates"][splitByColumns[0]] = splitByColumns[1];
  }
  //team b updates
  for(int i = 0; i < (int)teamBSplitByRow.size(); i++){
    splitByColumns = split(teamBSplitByRow[i],':');
    allGameUpdates["team b updates"][splitByColumns[0]] = splitByColumns[1];
  }
  return allGameUpdates;
}

void StompProtocol::clear(){
  username = "";
  userBySubscriptionId.erase(subscriptionIdByUser[username]);
  subscriptionIdByUser.erase(username);
  gamesByUser.erase(username);
  connected = false;
}

void StompProtocol::removeGameFromUser(std::string user, std::string game){
  for(int i = 0; i < (int)gamesByUser[user].size(); i++){
    if(gamesByUser[user][i] == game){
      gamesByUser[user].erase(gamesByUser[user].begin() + i);
    }
  }
}