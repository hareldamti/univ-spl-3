package bgu.spl.net.impl.stomp;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.AbstractMap.SimpleEntry;
import bgu.spl.net.srv.Connections;

public class Router {
    Frame request;
    ConnectionsImpl connections;
    int connectionId;


    public Router(Frame request, Connections<String> connections, int connectionId) {
        this.request = request;
        this.connections = (ConnectionsImpl)connections;
        this.connectionId = connectionId;
    }

    public Frame routeRequest() {
        switch(request.command){
            case CONNECT: return Connect();
            case DISCONNECT: return Disconnect();
            case SUBSCRIBE: return Subscribe();
            case UNSUBSCRIBE: return Unsubscribe();
            case SEND: return Send();
            default: return null;
        }
    }


    public Frame Connect() {
        String reqUsername = request.headers.get(Frame.HeaderKey.login),
            reqPasscode = request.headers.get(Frame.HeaderKey.passcode),
            reqAcceptVersion = request.headers.get(Frame.HeaderKey.accept_version);
        
        // check headers
        if (reqUsername == null || reqPasscode == null || reqAcceptVersion == null)
        {
            return Frame.createErrorFrame(request, "Missing headers",
            "Headers 'username', 'passcode', 'accept-version' are required for the CONNECT frame");
        }

        // create new user if needed
        String realPasscode = connections.userPassword.get(reqUsername);
        if (realPasscode == null) {
            connections.userPassword.put(reqUsername, reqPasscode);
        }

        // check password
        else if (!realPasscode.equals(reqPasscode)) {
            return Frame.createErrorFrame(request, "Password incorrect",
            "Please login again with the correct password, or create a new user");
        }

        // check if the client is already connected
        if (Utils.getKeyByValue(connections.userConnId, connectionId) != null)
        {
            return Frame.createErrorFrame(request, "Client already connected",
            "Disconnecting active client session. You may login again");
        }

        // check if the user is connected with another client
        if (connections.userConnId.get(reqUsername) != null)
        {
            return Frame.createErrorFrame(request, "User connected",
            "Logout from your active session before logging in again");
        }

        connections.userConnId.put(reqUsername, connectionId);
        
        Frame receipt = new Frame(Frame.Command.CONNECTED);
        receipt.headers.put(Frame.HeaderKey.version, request.headers.get(Frame.HeaderKey.accept_version));
        
        String receipt_id;
        if ((receipt_id = request.headers.get(Frame.HeaderKey.receipt_id)) != null) {
            receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        }
        return receipt;
    };

    public Frame Disconnect() {
        // check receipt header
        if (request.headers.get(Frame.HeaderKey.receipt_id) == null)
            return Frame.createErrorFrame(request, "Missing header",
            "In order to gracefully disconnect next time, send a DISCONNECT frame along with\n"+
            "a receipt-id and ensure accepting a matching RECEIPT");

        Frame receipt = new Frame(Frame.Command.RECEIPT);
        receipt.terminate = true;
        receipt.headers.put(Frame.HeaderKey.receipt_id, request.headers.get(Frame.HeaderKey.receipt_id));
        return receipt;
    };

    public Frame Subscribe() {
        // check if the client's user is logged in
        if (Utils.getKeyByValue(connections.userConnId, connectionId) == null)
        {
            return Frame.createErrorFrame(request, "Not logged in",
            "Log in before joining channels");
        }

        String username = Utils.getKeyByValue(connections.userConnId, connectionId),
            destination = request.headers.get(Frame.HeaderKey.destination),
            receipt_id = request.headers.get(Frame.HeaderKey.receipt_id);
        Integer subscription_id = Integer.parseInt(request.headers.get(Frame.HeaderKey.id));
        
        List<SimpleEntry<String, Integer>> subs;
        synchronized (connections.channelSubscriptions) {
            subs = connections.channelSubscriptions.get(destination);
            if (subs == null) {
                subs = Collections.synchronizedList(new ArrayList<SimpleEntry<String, Integer>>());
                connections.channelSubscriptions.put(destination, subs);
            }
        }
        subs.add(new SimpleEntry<String,Integer>(username, subscription_id));

        if (receipt_id == null) return null;
        
        Frame receipt = new Frame(Frame.Command.RECEIPT);
        receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        return receipt;
    }

    public Frame Unsubscribe() {
        // check if the client's user is logged in
        if (Utils.getKeyByValue(connections.userConnId, connectionId) == null)
        {
            return Frame.createErrorFrame(request, "Not logged in",
            "Log in before exiting channels");
        }
        
        String username = Utils.getKeyByValue(connections.userConnId, connectionId),
        receipt_id = request.headers.get(Frame.HeaderKey.receipt_id);
        Integer subscription_id = Integer.parseInt(request.headers.get(Frame.HeaderKey.id));
        SimpleEntry<String, Integer> userSub = new SimpleEntry<String, Integer>(username, subscription_id);
        for (List<SimpleEntry<String, Integer>> channelSubs : connections.channelSubscriptions.values()) 
        {
            SimpleEntry<String, Integer> matchedSub = null;
            for (SimpleEntry<String, Integer> sub : channelSubs) 
                if (sub.equals(userSub)) matchedSub = sub;
            if (matchedSub != null) channelSubs.remove(matchedSub);
        }
        
        if (receipt_id == null) return null;
        
        Frame receipt = new Frame(Frame.Command.RECEIPT);
        receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        return receipt;
    }
    
    public Frame Send() {
        // check if the client's user is logged in
        if (Utils.getKeyByValue(connections.userConnId, connectionId) == null)
        {
            return Frame.createErrorFrame(request, "Not logged in",
            "Log in before sending updates");
        }
        
        String username = Utils.getKeyByValue(connections.userConnId, connectionId),
            channel = request.headers.get(Frame.HeaderKey.destination),
            receipt_id = request.headers.get(Frame.HeaderKey.receipt_id);
        String messageBody = request.body;
        int messageId = connections.generateMessageId();
        List<SimpleEntry<String, Integer>> subs = connections.channelSubscriptions.get(channel);
        
        boolean userSubscribed = false;
        if (channel == null) {
            return Frame.createErrorFrame(request,
            "Channel doesn't exist", "Change destination header for an existing");
        }

        for (SimpleEntry<String, Integer> sub : subs)
            if (sub.getKey() == username) userSubscribed = true; 
        if (!userSubscribed) {
            return Frame.createErrorFrame(request,
            "User not subscribed", "Send a SUBSCRIBE frame to the desired channel first");
        }
        
        Frame broadcast = new Frame(Frame.Command.MESSAGE);
        broadcast.headers.put(Frame.HeaderKey.destination, channel);
        broadcast.headers.put(Frame.HeaderKey.message_id, String.valueOf(messageId));
        broadcast.body = messageBody;
        for (SimpleEntry<String, Integer> sub : subs) {
            broadcast.headers.put(Frame.HeaderKey.id, String.valueOf(sub.getValue()));
            connections.send( connections.userConnId.get(sub.getKey()), broadcast.toStringRepr() );
        }

        if (receipt_id == null) return null;
        
        Frame receipt = new Frame(Frame.Command.RECEIPT);
        receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        return receipt;
    }
}
