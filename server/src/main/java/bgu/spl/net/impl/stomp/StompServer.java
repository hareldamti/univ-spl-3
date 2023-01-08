package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {   
        Connections<String> connections = new ConnectionsImpl();
        int port;
        boolean reactorMode;
        try {
            port = Integer.parseInt(args[0]);
            if (args[1].toLowerCase() == "reactor") reactorMode = true;
            else if (args[1].toLowerCase() == "tpc") reactorMode = false;
            else throw new IllegalArgumentException("Illegal server mode");
        }
        catch (Exception ex) { System.out.println("Restart with arguments {port} {tpc/reactor}"); return; }

        if (reactorMode) {
            Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                port,
                () ->  new StompFrameProtocol(connections), //protocol factory
                FrameMessageEncoderDecoder::new //message encoder decoder factory
            ).serve();
        }

        else {
            Server.threadPerClient(
                    port,
                    () -> new StompFrameProtocol(connections), //protocol factory
                    FrameMessageEncoderDecoder::new //message encoder decoder factory
            ).serve();
        }
        



    }
}
