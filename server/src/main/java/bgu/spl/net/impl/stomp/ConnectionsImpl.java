package bgu.spl.net.impl.stomp;

import java.sql.Connection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl<T> implements Connections<T>{
    
    // Data structures: Users, Subscriptions, Connections-users, connectionId counter
    // Maps:
    //  users-passwords
    //  users-[subscription_id] 
    //  [subscription_id]-destinations
    //  user-connection_id
    //  connection_id-client

    public ConcurrentHashMap<String,String> users;
    public ConcurrentHashMap<String,ArrayList<Integer>> channels_connectionIds;
    public ConcurrentHashMap<Integer ,ConnectionHandler<T>> connectionHandlers;
    public ConcurrentHashMap<Integer,String> connenctions_users;

    public ConnectionsImpl(){
        users = new ConcurrentHashMap<>();
        channels_connectionIds = new ConcurrentHashMap<>();
        connenctions_users = new ConcurrentHashMap<>();
        connectionHandlers = new ConcurrentHashMap<>();

    }
    

    @Override
    public boolean send(int connectionId, T msg) {
        
        //uses the connectionhandler send function to send msg to a client

        if(connectionHandlers.get(connectionId).send(msg)) return true;
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        
        //sends a msg to all subscribed clients of a channel

        if(channels_connectionIds.keySet().contains(channel)){
            for(ConnectionHandler<T> client: n.get(channels_connectionIds.get(channel))){
                send(client.getConnectionId(), msg);
            }
        }
        
        
    }

    @Override
    public void disconnect(int connectionId) {
        //TODO Unsubscribe the client from the channels

        
        connenctions_users.remove(connectionId);
        connectionHandlers.remove(connectionId);
        
    }
    
}
