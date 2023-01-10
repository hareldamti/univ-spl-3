package bgu.spl.net.impl.stomp;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.AbstractMap.SimpleEntry;
import java.util.Map.Entry;
import java.util.concurrent.Callable;

import bgu.spl.net.impl.stomp.Frame.HeaderKey;
import bgu.spl.net.srv.Connections;

public class CommandRouter {
    Frame commandFrame;
    ConnectionsImpl connections;
    int connectionId;


    public CommandRouter(Frame commandFrame, Connections<String> connections, int connectionId) {
        this.commandFrame = commandFrame;
        this.connections = (ConnectionsImpl)connections;
        this.connectionId = connectionId;
    }

    public Frame Connect() {
        String reqUsername = commandFrame.headers.get(Frame.HeaderKey.login),
            reqPasscode = commandFrame.headers.get(Frame.HeaderKey.passcode),
            reqAcceptVersion = commandFrame.headers.get(Frame.HeaderKey.accept_version);
        
        if (reqUsername == null || reqPasscode == null || reqAcceptVersion == null)
        {
            return Frame.createErrorFrame(commandFrame, "Missing headers",
            "Headers 'username', 'passcode', 'accept-version' are required for the CONNECT frame");
        }

        String realPasscode = connections.userPassword.get(reqUsername);
        if (realPasscode == null) {
            connections.userPassword.put(reqUsername, reqPasscode);
        }

        if (Utils.getKeyByValue(connections.userConnId, connectionId) != null)
        {
            return Frame.createErrorFrame(commandFrame, "Client already connected",
            "Disconnecting active client session. You may login again");
        }

        if (connections.userConnId.get(reqUsername) != null)
        {
            return Frame.createErrorFrame(commandFrame, "User connected",
            "Logout from your active session before logging in again");
        }
        connections.userConnId.put(reqUsername, connectionId);

        Frame receipt = new Frame(Frame.Command.CONNECTED);
        receipt.headers.put(Frame.HeaderKey.version,
        commandFrame.headers.get(Frame.HeaderKey.accept_version));
        
        String receipt_id;
        if ((receipt_id = commandFrame.headers.get(Frame.HeaderKey.receipt_id)) != null) {
            receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        }
        return receipt;
    };

    public Frame Disconnect() {
        if (commandFrame.headers.get(Frame.HeaderKey.receipt_id) == null)
            return Frame.createErrorFrame(commandFrame, "Missing header",
            "In order to gracefully disconnect, send a DISCONNECT frame along with\n"+
            "a receipt-id and ensure accepting a matching RECEIPT");

        Frame receipt = new Frame(Frame.Command.RECIEPT);
        receipt.terminate = true;
        receipt.headers.put(Frame.HeaderKey.receipt_id, commandFrame.headers.get(Frame.HeaderKey.receipt_id));
        return receipt;
    };

    public Frame Subscribe() {
        String username = Utils.getKeyByValue(connections.userConnId, connectionId),
            destination = commandFrame.headers.get(Frame.HeaderKey.destination),
            receipt_id = commandFrame.headers.get(Frame.HeaderKey.receipt_id);
        Integer subscription_id = Integer.parseInt(commandFrame.headers.get(Frame.HeaderKey.id));
        
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
        
        Frame receipt = new Frame(Frame.Command.RECIEPT);
        receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        return receipt;
    }

    public Frame Unsubscribe() {
        String username = Utils.getKeyByValue(connections.userConnId, connectionId),
        receipt_id = commandFrame.headers.get(Frame.HeaderKey.receipt_id);
        Integer subscription_id = Integer.parseInt(commandFrame.headers.get(Frame.HeaderKey.id));
        SimpleEntry<String, Integer> userSub = new SimpleEntry<String, Integer>(username, subscription_id);
        for (List<SimpleEntry<String, Integer>> channelSubs : connections.channelSubscriptions.values()) 
        {
            SimpleEntry<String, Integer> matchedSub = null;
            for (SimpleEntry<String, Integer> sub : channelSubs) 
                if (sub.equals(userSub)) matchedSub = sub;
            if (matchedSub != null) channelSubs.remove(matchedSub);
        }
        
        if (receipt_id == null) return null;
        
        Frame receipt = new Frame(Frame.Command.RECIEPT);
        receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        return receipt;
    }
    
    public Frame Send() {
        String username = Utils.getKeyByValue(connections.userConnId, connectionId),
            channel = commandFrame.headers.get(Frame.HeaderKey.destination),
            receipt_id = commandFrame.headers.get(Frame.HeaderKey.receipt_id);
        String messageBody = commandFrame.body;
        int messageId = connections.generateMessageId();
        List<SimpleEntry<String, Integer>> subs = connections.channelSubscriptions.get(channel);
        
        boolean userSubscribed = false;
        if (channel == null) {
            return Frame.createErrorFrame(commandFrame,
            "Channel doesn't exist", "Change destination header for an existing");
        }

        for (SimpleEntry<String, Integer> sub : subs)
            if (sub.getKey() == username) userSubscribed = true; 
        if (!userSubscribed) {
            return Frame.createErrorFrame(commandFrame,
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
        
        Frame receipt = new Frame(Frame.Command.RECIEPT);
        receipt.headers.put(Frame.HeaderKey.receipt_id, receipt_id);
        return receipt;
    }
}
