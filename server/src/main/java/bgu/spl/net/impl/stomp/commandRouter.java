package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.concurrent.Callable;

import bgu.spl.net.srv.Connections;

public class CommandRouter {
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
        throw new NoSuchFieldException();
    };
    static Callable<Frame> Disconnect = () -> {
        throw new NoSuchFieldException();
    };
    static Callable<Frame> Subscribe = () -> {
        throw new NoSuchFieldException();
    };
    static Callable<Frame> Unsubscribe = () -> {
        throw new NoSuchFieldException();
    };
    static Callable<Frame> Send = () -> {
        throw new NoSuchFieldException();
    };
}
