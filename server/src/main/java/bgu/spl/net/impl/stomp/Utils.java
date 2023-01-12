package bgu.spl.net.impl.stomp;

import java.util.Map;
import java.util.Map.Entry;

public class Utils {
    public enum LogLevel {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        OFF
    }
    public static final String DEFAULT_HOST = "stomp.cs.bgu.ac.il";
    public static final LogLevel loggingLevel = LogLevel.OFF;
    
    

    public static void log(Object o, LogLevel level) {
        if (loggingLevel.ordinal() <= level.ordinal())
            System.out.printf(
                "%s\t%s\t%s\n",
                java.time.LocalDateTime.now(),
                level.name(),
                o.toString()
            );
    }

    public static void log(Object o) {
        log(o, LogLevel.INFO);
    }
    
    public static void log(Exception e) {
        if (loggingLevel.ordinal() <= LogLevel.ERROR.ordinal()) {
            System.out.printf(
                "%s\t%s\n",
                java.time.LocalDateTime.now(),
                LogLevel.ERROR.name());
            e.printStackTrace();
        }
    }

    public static <K,V> K getKeyByValue(Map<K,V> map, V value) {
        for (Entry<K, V> pair : map.entrySet()) 
            if (pair.getValue().equals(value))
                return pair.getKey();
        return null;
    }
}
