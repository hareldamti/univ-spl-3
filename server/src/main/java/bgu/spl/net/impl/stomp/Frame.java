package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class Frame {
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



    public String raw_frame;
    public String command;
    public Map<String, String> headers;
    public String body;

    public boolean isCorrupted;


    public Frame(String raw_frame){
        isCorrupted = false;
        headers = new HashMap<String, String>();
        this.raw_frame = raw_frame;
        parseFrame(raw_frame);
    }

    public Frame(String command, Map<String, String> headers, String body){
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
        String[] lines = raw_frame.split("\n");
        if (lines.length == 0) {
            isCorrupted = true;
            return;
        }

        command = lines[0];

        int lineIdx = 1;
        while (lineIdx < lines.length && lines[lineIdx].length() != 0){
            String currentLine = lines[lineIdx];
            String[] keyValuePair = currentLine.split(":");
            if (keyValuePair.length != 2) 
                isCorrupted = true;
            else headers.put(keyValuePair[0], keyValuePair[1]);
            lineIdx++;
        }
        
        body = "";
        while (lineIdx < lines.length) {
            body += lines[lineIdx] + (lineIdx == lines.length - 1 ? "" : "\n");
        }
    }

    private String serializeString() {
        String raw_frame = command + "\n";
        for (String key : headers.keySet()) {raw_frame += String.format("%s:%s\n",key, headers.get(key));}
        raw_frame += "\n" + body + "\u0000";
        return raw_frame;
    }

}
