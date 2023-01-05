package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class Frame {
    public String raw_frame;
    public Command command;
    public Map<Header, String> headers;
    public String body;

    public boolean isCorrupted;


    public Frame(String raw_frame){
        isCorrupted = false;
        headers = new HashMap<Header, String>();
        this.raw_frame = raw_frame;
        parseFrame(raw_frame);
    }

    public Frame(Command command, Map<Header, String> headers, String body){
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    /**
     * Injects the data from raw_frame to this.
     * In case of a severely corrupted message, raises a corrupted flag
     * @param raw_frame
     */
    private void parseFrame(String raw_frame){
        String errorSummary = "", errorMessage = "";
        String[] lines = raw_frame.split("\n");

        if (lines.length == 0)
            isCorrupted = true;


        try {command = Command.valueOf(lines[0]); }
        catch (IllegalArgumentException notInEnum) {
            isCorrupted = true;
            errorSummary = "Illegal command";
            errorMessage = "Notice to use capitals and either\n"+
            "Un\\Subscribe, Dis\\Connect, Send";
        }

        int lineIdx = 1;
        while (lineIdx < lines.length && lines[lineIdx].length() != 0){
            String currentLine = lines[lineIdx];
            String[] keyValuePair = currentLine.split(":");
            if (keyValuePair.length != 2) 
                isCorrupted = true;
            else {
                try {
                    headers.put(
                        parseHeader(keyValuePair[0]),
                        keyValuePair[1].replace(" ","")
                        );
                } catch (IllegalArgumentException notInEnum) {
                    isCorrupted = true;
                    errorSummary = "Illegal header";
                    errorMessage = "Notice to use a key value pair,\n"+
                    "seperated by :, and a relevant key (found in manual)";
                }

            }
            lineIdx++;
        }
        
        body = "";
        while (lineIdx < lines.length) {
            body += lines[lineIdx] + (lineIdx == lines.length - 1 ? "" : "\n");
        }

        if (isCorrupted) this = createErrorFrame(this, errorSummary, errorMessage);
    }

    private String toStringRepr() {
        String raw_frame = command + "\n";
        for (Header key : headers.keySet()) {raw_frame += String.format("%s:%s\n",headerName(key), headers.get(key));}
        raw_frame += "\n" + body + "\u0000";
        return raw_frame;
    }

    public static Frame createErrorFrame(Frame frame, String errorSummary, String errorMessage) {
        Map<Header, String> headers = new HashMap<Header, String>();
        headers.put(Header.message, errorSummary);
        if (frame.headers.containsKey(Header.receipt_id))
            headers.put(Header.receipt, frame.headers.get(Header.receipt_id));
        
        String body = "The message\n-----\n" + frame.raw_frame + "\n-----\n" + errorMessage;
        return new Frame(Command.ERROR, headers, body);
    }

    /// Enums:

    public enum Command {
        ERROR,
        CONNECT,
        DISCONNECT,
        CONNECTED,
        SUBSCRIBE,
        UNSUBSCRIBE,
        RECIEPT,
        SEND
    }

    public enum Header {
        receipt_id,
        subscription,
        message_id,
        destination,
        id,
        accept_version,
        version,
        login,
        host,
        message,
        password,
        receipt
    }

    public static String headerName(Header h) {
        return h.name().replace("_", "-");
    }

    public static Header parseHeader(String s) {
        return Header.valueOf(s.replace("-", "_"));
    }
}
