package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.Map;

import static java.util.Map.entry;  

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;



public class StompFrameProtocol implements StompMessagingProtocol<String> {

    //TODO: type??

    int connectionId;
    Connections<String> connections;
    public StompFrameProtocol(Connections<String> connections) { this.connections = connections; }

    @Override
    public void start(ConnectionHandler<String> conn) {
        this.connectionId = connections.generateUniqueId(conn);
    }
    
    @Override
    public void process(String message) {
        //TODO: channel?
        Frame request = Frame.parseFrame(message);
        if (request.isCorrupted) {
            connections.send(connectionId, request.toStringRepr());
        }

        CommandRouter router = new CommandRouter(request, connections);
        try {
            Frame response = router.getCommand().call();
            connections.send(connectionId, response.toStringRepr()); // A receipt message
        }
        catch (Exception e) {
            Utils.log("Processing failed\n\nrequest:\n"+message+e.toString(),
            Utils.LogLevel.ERROR);
        }
        
        
        
    }
	
	/**
	 * @return 
     */
    @Override
    public boolean shouldTerminate() {
        return false;
        //TODO: implement
    }


    




}