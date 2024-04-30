#include <iostream>
#include "../include/StompProtocol.h"
using namespace std;

int main(int argc, char *argv[]) {
	while (1) {
		// TODO: implement the STOMP client
		cout << "Please login:\n";
		std::string loginDetails;
		vector<std::string> loginDetailsVector;
		vector<std::string> hostAndPortVector;
		getline(cin, loginDetails);
		loginDetailsVector = split(loginDetails, ' ');
		hostAndPortVector = split(loginDetailsVector[1], ':');
		std::string host = hostAndPortVector[0];
		short port = (short)stoi(hostAndPortVector[1]);
		std::string username = loginDetailsVector[2];
		std::string password = loginDetailsVector[3];
		//create the connection handler and connect to the server
		ConnectionHandler connectionHandler(host, port);
		if (!connectionHandler.connect()) {
			std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
			std::cout << "Couldn't connect to server\n" << std::endl;/////////////////
			continue;
		}
		//create the stomp protocol
		StompProtocol stompProtocol(&connectionHandler, username);
		//create the login frame and send it to the server
		std::string loginFrame = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + stompProtocol.getUsername() + "\npasscode:" + password + "\n\n\0";
		if (!connectionHandler.sendLine(loginFrame)) {
			std::cout << "Could not send login frame to server\n" << endl;
			std::cout << "Disconnected. Exiting...\n" << std::endl;
			continue;
		}
		//Wait for the server to send the connected frame
		std::string answer;
		if (!connectionHandler.getLine(answer)) {
			std::cout << "Disconnected. Exiting...\n" << std::endl;
			continue;
		}
		//check if the server sent the connected frame
		if (answer.find("CONNECTED") == std::string::npos) {
			std::cout << "Error: the server did not send the connected frame. Exiting...\n" << std::endl;
			continue;
		}
		std::cout << "Login successful" << std::endl;
		stompProtocol.addSubscriptionId(username);
		stompProtocol.setConnected();
		
		//create the thread that will read from the keyboard
		std::thread t1([](StompProtocol& stompProtocol) {
			while (stompProtocol.isConnected()) {
				std::string line = getInputFromUser(); // Get the line from the keyboard
				if (!stompProtocol.isConnected()){
					
					break;
				}
				stompProtocol.processFromUser(line); // Process the line from the keyboard
			}
		}, std::ref(stompProtocol));

		//Read messages from the server
		while(stompProtocol.isConnected()){
			stompProtocol.processFromServer();
		}
		t1.join();

	}
}