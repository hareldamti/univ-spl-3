package bgu.spl.net.impl.stomp;

import java.sql.Connection;

import bgu.spl.net.srv.Connections;

public class ConnectionsImpl<T> implements Connections<T>{

    @Override
    public boolean send(int connectionId, T msg) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void disconnect(int connectionId) {
        // TODO Auto-generated method stub
        
    }
    
}
