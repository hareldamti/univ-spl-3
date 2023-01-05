package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.Map;

import static java.util.Map.entry;  

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;



public class StompFrameProtocol implements StompMessagingProtocol<String> {

    //TODO: type??

    int connectionId;
    Connections<String> connections;
    public StompFrameProtocol(Connections<String> connections) {
        this.connections = connections;
        //TODO: implement
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        // TODO Auto-generated method stub
        this.connectionId = connectionId;
        this.connections = connections;
    }
    
    @Override
    public void process(String message) {
        //TODO: channel?
        Frame request = Frame.parseFrame(message);
        if (request.isCorrupted) {
            
            connections.send(null, request.toStringRepr());
        }

        CommandRouter router = new CommandRouter(request, connections);
        Frame response;
        try { response = router.getCommand().call(); }
        catch (Exception e) {}
        
        // A receipt message
        connections.send(null, request.toStringRepr());
    }
	
	/**
	 * @return 
     */
    @Override
    public boolean shouldTerminate() {
        return false;
        //TODO: implement
    }

    public Frame handleCommand(Frame request) {
        response =
    }

    




}