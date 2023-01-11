package bgu.spl.net.impl.stomp;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.AbstractMap.SimpleEntry;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl implements Connections<String>{
    /**
     * username: password dictionary
     */
    public ConcurrentHashMap<String, String> userPassword;

    /**
     * channel: [(username, subscriptionId)] dictionary
     */
    public ConcurrentHashMap<String,List<SimpleEntry<String, Integer>>>
        channelSubscriptions;

    /**
     * connectionId: connectionHandler dictionary
     */
    public ConcurrentHashMap<Integer, ConnectionHandler<String>> connIdHandler;

    /**
     * username: connectionId dictionary
     */
    public ConcurrentHashMap<String,Integer> userConnId;

    private int connIdCounter, msgIdCounter;

    public ConnectionsImpl() {
        userPassword = new ConcurrentHashMap<>();
        channelSubscriptions = new ConcurrentHashMap<>();
        connIdHandler = new ConcurrentHashMap<>();
        userConnId = new ConcurrentHashMap<>();
        connIdCounter = 0;
        msgIdCounter = 0;
    }
    
    /**
     * Finds the connectionHandler and sends a message through it
     * @param connectionId: client's unique connectionId
     * @param msg: message to be sent
     * @return: true iff the connectionHandler returned a succesful send
     */
    @Override
    public boolean send(int connectionId, String msg) {
        try {
            if (connIdHandler.get(connectionId).send(msg + "\u0000")) return true;
        }
        catch (NullPointerException e) {
            Utils.log("Connection id "+connectionId+" isn't assigned to a handler",
            Utils.LogLevel.ERROR);
        }
        return false;
    }

    /**
     * Finds the subscribed users' connectionHandlers and sends a message to all of them
     * @param channel: specific channel subscription
     * @param msg: message to be sent
    */
    @Override
    public void send(String channel, String msg) {
        List<SimpleEntry<String, Integer>> subscriptions = channelSubscriptions.get(channel);
        if (subscriptions == null) {
            Utils.log("channel "+channel+" was not registered but a message was sent to it",
            Utils.LogLevel.ERROR);
            return;
        }
        if (subscriptions.size() == 0) {
            Utils.log("channel "+channel+" has 0 subs but a message was sent to it",
            Utils.LogLevel.WARNING);
            return;
        }
        Frame msgFrame = Frame.parseFrame(msg);
        for (SimpleEntry<String, Integer> sub : subscriptions) {
            String username = sub.getKey();
            Integer subscriptionId = sub.getValue();
            msgFrame.headers.put(Frame.HeaderKey.subscription, subscriptionId.toString());
            Integer connectionId = userConnId.get(username);
            if (connectionId == null) {
                Utils.log("user "+username+" is subscribed to "+channel+" but isn\'t logged in",
                Utils.LogLevel.INFO);
            }
            else send(connectionId, msgFrame.toStringRepr());
        }  
    }

    @Override
    public void disconnect(int connectionId) {

        String username = Utils.getKeyByValue(userConnId, connectionId);
        if (username != null) {
            // remove user's subscriptions
            for (String channel : channelSubscriptions.keySet()) {
                List<SimpleEntry<String, Integer>> subs = channelSubscriptions.get(channel);
                List<SimpleEntry<String, Integer>> userSubs = new ArrayList<SimpleEntry<String, Integer>>();
                for (SimpleEntry<String, Integer> sub : subs) 
                    if (sub.getKey() == username) 
                        userSubs.add(sub);
                
                for (SimpleEntry<String, Integer> userSub : userSubs)
                    subs.remove(userSub);
            }
            // remove user's current connection id
            userConnId.remove(username);
        }
    }


    @Override
    public int generateUniqueConnectionId(ConnectionHandler<String> conn) {
        connIdHandler.put(connIdCounter, conn);
        return connIdCounter++;
    }

    public int generateMessageId() { return msgIdCounter++; }

    @Override
    public void kill(int connectionId) {
        connIdHandler.remove(connectionId);
    }
    
}
