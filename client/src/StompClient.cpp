#include <iostream>
#include "../include/ConnectionHandler.h"
#include "../include/ClientIO.h"
#include <vector>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
    while(1){
        ClientIO* clio = new ClientIO();
    
        if (!clio->startConnection()){delete clio; return 1;}

        std::thread thread_object(&ClientIO::sendRequests, &(*clio));
        clio->processMessages();
        thread_object.join();
        delete clio;
    }
	return 0;
}