#include <iostream>
#include "ConnectionHandler.h"

class ClientIO {
	bool terminated;
	Queue<string> requestQueue;
	ClientIO() : terminated(false), requestQueue(0){}

	void registerInputs() {
		while (!terminate) {
			const short bufsize = 1024;
			char buf[bufsize];
			std::cin.getline(buf, bufsize);
			std::string line(buf);
			int len=line.length();
		}
	}

	void displayResponses() {
		while (!terminate) {
			// lockguard - mutex - lock queue - read inputs - send - wait for response - process = display

			if (!connectionHandler.sendLine(line)) {
				std::cout << "Disconnected. Exiting...\n" << std::endl;
				terminate = true;
			}
		}
	}

}


int main(int argc, char *argv[]) {
	// TODO: implement the STOMP client
	if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
	std::string host = argv[1];
    short port = atoi(argv[2]);
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
	std::cout << "Connected to the server" << std::endl;


	return 0;
}