#include <iostream>
#include "ConnectionHandler.h"
#include "ClientIO.h"
#include <vector>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
    ClientIO clio = new ClientIO();

    if (!clio.startConnection()) return 1;

	std::thread thread_object(ClientIO::sendRequests, clio);
    clio->displayMessages(connectionHandler);
    connectionHandler.close();
	return 0;
}