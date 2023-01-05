package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.Map;

import static java.util.Map.entry;  

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;



public class StompFrameProtocol implements StompMessagingProtocol<String> {

    //TODO: type??

    Connections connections;
    public StompFrameProtocol(Connections connections) {
        this.connections = connections;
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        // TODO Auto-generated method stub
        
    }
    
    @Override
    public void process(String message) {
        //TODO: implement
        Frame request = new Frame(message);
        if (request.isCorrupted) {
            //TODO
        }
        Frame response = handleCommand(request);
    }
	
	/**
	 * @return 
     */
    @Override
    public boolean shouldTerminate() {
        return false;
        //TODO: implement
    }
    
    public Frame isLegal(Frame request) {
        if (request.isCorrupted) return createErrorFrame(request, "Unreadable headers", "");
        //TODO: implement
    }


    public Frame handleCommand(Frame request) {
        switch(request.command) {
            case "CONNECT": {

            }
            case "SUBSCRIBE": {
                
            }
            case "SUBSCRIBE": {
                
            }
            case "SUBSCRIBE":
        }

        return null;
    }

    




}