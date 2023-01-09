package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class Frame {
    public Command command;
    public Map<HeaderKey, String> headers;
    public String body;

    public boolean terminate;

    public Frame(Command command){
        this.command = command;
        this.headers = new HashMap<HeaderKey, String>();
        this.body = "";
    }
    public Frame(Command command, Map<HeaderKey, String> headers, String body){
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    /**
     * Injects the data from raw_frame to a new Frame object.
     * In case of a severely corrupted message, returns an ERROR Frame.
     * 
     * @param raw_frame: string representation of the received frame
     */
    public static Frame parseFrame(String rawFrame){
        Frame result = new Frame(null, new HashMap<HeaderKey, String>(), "");
        result.terminate = false;
        String errorSummary = "", errorMessage = "";
        String[] lines = rawFrame.split("\n");

        if (lines.length == 0) {
            result.terminate = true;
            errorSummary = "Empty message";
        }

        try {result.command = Command.valueOf(lines[0]); }
        catch (IllegalArgumentException notInEnum) {
            result.terminate = true;
            errorSummary = "Illegal command";
            errorMessage = "Notice to use capitals and either\n"+
            "Un\\Subscribe, Dis\\Connect, Send";
        }

        int lineIdx = 1;
        while (lineIdx < lines.length && lines[lineIdx].length() != 0){
            String currentLine = lines[lineIdx];
            String[] keyValuePair = currentLine.split(":");
            if (keyValuePair.length != 2) 
                result.terminate = true;
            else {
                try {
                    result.headers.put(
                        parseHeader(keyValuePair[0]),
                        keyValuePair[1].replace(" ","")
                        );
                } catch (IllegalArgumentException notInEnum) {
                    result.terminate = true;
                    errorSummary = "Illegal header";
                    errorMessage = "Notice to use a key value pair,\n"+
                    "seperated by :, and a relevant key (found in manual)";
                }
            }
            lineIdx++;
        }
        
        while (lineIdx < lines.length) {
            result.body += lines[lineIdx] + (lineIdx == lines.length - 1 ? "" : "\n");
        }

        if (result.terminate)
            return createErrorFrame(result, errorSummary, errorMessage);
        return result;
    }

    public String toStringRepr() {
        String raw_frame = command + "\n";
        for (HeaderKey key : headers.keySet()) {raw_frame += String.format("%s:%s\n",headerName(key), headers.get(key));}
        raw_frame += "\n" + body + "\u0000";
        return raw_frame;
    }

    public static Frame createErrorFrame(Frame frame, String errorSummary, String errorMessage) {
        Map<HeaderKey, String> headers = new HashMap<HeaderKey, String>();
        headers.put(HeaderKey.message, errorSummary);
        if (frame.headers.containsKey(HeaderKey.receipt_id))
            headers.put(HeaderKey.receipt_id, frame.headers.get(HeaderKey.receipt_id));
        String body = "The message\n-----\n" + frame.toStringRepr() + "\n-----\n" + errorMessage;
        Frame response = new Frame(Command.ERROR, headers, body);
        response.terminate = true;
        return response;
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
        SEND,
        MESSAGE
    }

    public enum HeaderKey {
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
        passcode,
    }

    public static String headerName(HeaderKey h) {
        return h.name().replace("_", "-");
    }

    public static HeaderKey parseHeader(String s) {
        return HeaderKey.valueOf(s.replace("-", "_"));
    }
}
