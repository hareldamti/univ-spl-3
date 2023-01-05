package bgu.spl.net.impl.stomp;
import java.util.Dictionary;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.Callable;
import java.util.concurrent.Flow.Subscriber;

import bgu.spl.net.srv.Connections;

public class commandRouter{
    Frame commandFrame;
    Connections connections;
    HashMap<Consts.Command, Callable<Frame>> commandDictionary = new HashMap<Consts.Command, Callable<Frame>>(){{
        put(Consts.Command.CONNECT, Connect);
        put(Consts.Command.DISCONNECT, Disconnect);
        put(Consts.Command.SUBSCRIBE, Subscribe);
        put(Consts.Command.UNSUBSCRIBE, Unsubscribe);
        put(Consts.Command.SEND, Send);
    }};


    public commandRouter(Frame commandFrame, Connections connections) {
        this.commandFrame = commandFrame;
        this.connections = connections;
    }
    public Callable<Frame> getCommand(Frame request) {
        return commandDictionary.get(request.command);
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
