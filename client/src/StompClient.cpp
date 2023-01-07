#include <iostream>
#include "ConnectionHandler.h"
#include "ClientIO.h"
#include <vector>
#include <string>
#include <thread>




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
	ClientIO *clio = new ClientIO();
	std::thread thread_object(ClientIO::registerInputs)
	


	return 0;
}