#pragma once

#include "../include/Game.h"
#include "../include/event.h"
#include "../include/ConnectionHandler.h"
#include <mutex>

// TODO: implement the STOMP protocol
class StompProtocol
{
    private:
        ConnectionHandler* connectionHandler;
        std::string username;
        int receiptIdGenerator;
        int subscriptionIdGenerator;
        bool connected;
        std::map<std::string, Game*> games;
        //subscription id by user
        std::map<std::string, std::vector<std::string>> gamesByUser;
        std::map<std::string, vector<std::string>> receiptIdToResponseData;
        std::map<std::string, std::string> subscriptionIdByUser;
        std::map<std::string, std::string> userBySubscriptionId;
    public:
        StompProtocol(ConnectionHandler* connectionHandler , std::string username);
        virtual ~StompProtocol();
        StompProtocol(const StompProtocol& other);
        //copy assignment operator
        StompProtocol& operator=(const StompProtocol& other);
        void processFromServer();
        void processFromUser(std::string line);
        std::tuple<std::string, std::map<std::string, std::string>, std::string> parseStompFrame(const std::string& frame);
        bool isConnected();
        void setConnected();
        void setDisconnected();
        void addGameToUser(std::string user, std::string game);
        std::vector<Game> getGamesByUser(std::string user);
        void addSubscriptionId(std::string user);
        std::string getSubscriptionId(std::string user);
        void removeSubscriptionId(std::string id);
        std::string getUsername();
        std::map<std::string, std::map<std::string, std::string>> parseMessage(std::string message);
        void removeGameFromUser(std::string user, std::string game);
        void clear();
};

std::vector<std::string> split(const std::string& s, char delimiter);
std::string getInputFromUser();

