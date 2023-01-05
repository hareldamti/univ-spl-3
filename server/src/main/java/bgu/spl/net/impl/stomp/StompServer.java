package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {   
        Map<String, String> users = new HashMap<String, String>();
        Connections<String> connections = new ConnectionsImpl<String>();
        Server.threadPerClient(
                7777, //port
                () -> new StompFrameProtocol(connections), //protocol factory
                FrameMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();

        Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                7777, //port
                () ->  new StompFrameProtocol(connections), //protocol factory
                FrameMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();
    }
}
