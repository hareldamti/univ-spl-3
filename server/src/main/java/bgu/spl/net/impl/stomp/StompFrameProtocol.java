package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.Map;

import static java.util.Map.entry;  

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;



public class StompFrameProtocol implements StompMessagingProtocol<String> {

    int connectionId;
    boolean terminate;
    Connections<String> connections;
    public StompFrameProtocol(Connections<String> connections) {
        this.connections = connections;
        this.terminate = false;
    }

    @Override
    public void start(ConnectionHandler<String> conn) {
        this.connectionId = connections.generateUniqueConnectionId(conn);
    }
    
    @Override
    public void process(String message) {
        Frame request = Frame.parseFrame(message);
        if (request.terminate) 
            connections.send(connectionId, request.toStringRepr());
        

        CommandRouter router = new CommandRouter(request, connections, connectionId);
        try {
            Frame response = router.getCommand().call();
            
            if (response != null) {
                if (response.terminate)
                    connections.disconnect(connectionId);
                
                connections.send(connectionId, response.toStringRepr()); // A receipt message
            }

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
        return terminate;
        
    }

    @Override
    public void close() {
        connections.kill(connectionId);
    }


    




}