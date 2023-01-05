package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.Map;

import static java.util.Map.entry;  

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;



public class StompFrameProtocol implements StompMessagingProtocol<String> {

    public StompFrameProtocol() {
        //TODO: implement
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        // TODO Auto-generated method stub
        
    }
    
    @Override
    public void process(String message) {
        //TODO: implement
        Frame request = new Frame(message);
        Frame error = isLegal(request);
        //TODO: handle error

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

    public Frame createErrorFrame(Frame frame, String errorSummary, String errorMessage) {
        Map<String, String> headers = new HashMap<String,String>();
        headers.put("message", errorSummary);
        if (frame.headers.containsKey("reciept-id")) headers.put("reciept-id", frame.headers.get("reciept-id"));
        String body = "The message\n-----\n" + frame.raw_frame + "\n-----\n"+errorMessage;
        return new Frame(Frame.Command.ERROR.name(), headers, body);
    }




}