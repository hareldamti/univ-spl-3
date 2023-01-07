package bgu.spl.net.impl.stomp;

public class Utils {
    public enum LogLevel {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        OFF
    }
    public static final String DEFAULT_HOST = "bgu.spl.ac.il";
    public static final LogLevel loggingLevel = LogLevel.INFO;
    
    

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
}
