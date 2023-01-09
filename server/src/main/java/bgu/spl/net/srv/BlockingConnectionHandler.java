package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.impl.stomp.StompFrameProtocol;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;



public class BlockingConnectionHandler implements Runnable, ConnectionHandler<String> {

    private final MessagingProtocol<String> protocol;
    private final MessageEncoderDecoder<String> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<String> reader, MessagingProtocol<String> protocol) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        ((StompFrameProtocol)protocol).start(this);
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { //just for automatic closing
            int read;
            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());
            
            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                String nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null) {
                    protocol.process(nextMessage);
                }
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }

    }

    @Override
    public void close() throws IOException {
        ((StompFrameProtocol)protocol).close();
        connected = false;
        sock.close();
    }

    @Override
    public boolean send(String msg) {
        boolean sent = true;
        try{
            out.write(encdec.encode(msg));
            out.flush();
        }
        catch(IOException e){
            sent = false;
        }
        return sent;
    }


}
