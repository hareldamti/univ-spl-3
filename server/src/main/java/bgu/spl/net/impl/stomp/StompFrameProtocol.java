package bgu.spl.net.impl.stomp;

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
        if (request.terminate) {
            connections.send(connectionId, request.toStringRepr());
            terminate = true;
        }

        Utils.log("--Received request--\n\nConnection Id:\t"+connectionId+"\nFrame:\n"+request.toStringRepr(), Utils.LogLevel.DEBUG);

        Router router = new Router(request, connections, connectionId);
        try {
            Frame response = router.routeRequest();

            if (response != null) {
                if (response.terminate) {
                    connections.disconnect(connectionId);
                    terminate = true;
                }
                connections.send(connectionId, response.toStringRepr()); // A receipt/error message
            }
        }
        catch (Exception e) {
            Utils.log("Processing failed\n\nrequest:\n"+message+"\nerror:\n"+e.toString(),
            Utils.LogLevel.ERROR);
        }
    }
	
    @Override
    public boolean shouldTerminate() {
        return terminate;
    }

    @Override
    public void close() {
        connections.kill(connectionId);
    }
}