package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {   
        Connections<String> connections = new ConnectionsImpl();
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
