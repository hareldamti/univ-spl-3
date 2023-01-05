package bgu.spl.net.impl.stomp;
import java.util.Dictionary;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.Callable;
import java.util.concurrent.Flow.Subscriber;

import bgu.spl.net.srv.Connections;

public class CommandRouter{
    Frame commandFrame;
    ConnectionsImpl connections;
    HashMap<Frame.Command, Callable<Frame>> commandDictionary = new HashMap<Frame.Command, Callable<Frame>>(){{
        put(Frame.Command.CONNECT, Connect);
        put(Frame.Command.DISCONNECT, Disconnect);
        put(Frame.Command.SUBSCRIBE, Subscribe);
        put(Frame.Command.UNSUBSCRIBE, Unsubscribe);
        put(Frame.Command.SEND, Send);
    }};


    public CommandRouter(Frame commandFrame, Connections<String> connections) {
        this.commandFrame = commandFrame;
        this.connections = (ConnectionsImpl)connections;
    }
    public Callable<Frame> getCommand() {
        return commandDictionary.get(commandFrame.command);
    }

    static Callable<Frame> Connect = () -> {

    };
    static Callable<Frame> Disconnect = () -> {
        
    };
    static Callable<Frame> Subscribe = () -> {
        
    };
    static Callable<Frame> Unsubscribe = () -> {
        
    };
    static Callable<Frame> Send = () -> {
        
    };
}
