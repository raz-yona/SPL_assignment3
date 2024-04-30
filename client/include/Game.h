#include "../include/event.h"
#include <mutex>
using namespace std;

//class Game
class Game
{
    private:
        std::string team_a_name;
        std::string team_b_name;
        std::map<std::string, vector<std::string>> events_by_user;
        std::map<std::string, std::map<std::string, string>> gameUpdates;
    public:
        Game(std::string teams_names);
        void add_event(std::string user, std::string event);
        std::vector<std::string> get_events_by_user(std::string user);
        std::string get_teams_names();
        void updateGame(std::map<std::string, std::map<std::string, string>> newGameUpdates);
        std::map<std::string, std::map<std::string, string>> getGameUpdates();

};