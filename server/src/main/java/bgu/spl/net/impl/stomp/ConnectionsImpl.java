package bgu.spl.net.impl.stomp;

import java.sql.Connection;

import bgu.spl.net.srv.Connections;

public class ConnectionsImpl implements Connections<String>{

    // Data structures: Users, Subscriptions, Connections-users, connectionId counter
    // Maps:
    //  users-passwords
    //  users-[subscription_id]
    //  [subscription_id]-destinations
    //  user-connection_id
    //  connection_id-client
    @Override
    public boolean send(int connectionId, String msg) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public void send(String channel, String msg) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void disconnect(int connectionId) {
        // TODO Auto-generated method stub
        
    }
    
}
