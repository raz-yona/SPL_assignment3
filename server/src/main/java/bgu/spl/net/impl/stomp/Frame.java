package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class Frame {

    private String command;
    private Map<String,String> headers;
    private String body;

    public Frame(String command, Map<String,String> headers, String body){
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    public String getCommand() {
        return command;
    }

    public Map<String, String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }

    public void addHeader(String key, String value){
        headers.put(key,value);
    }

    public static Frame stringToFrame(String frameString){
        String[] lines = frameString.split("\n");
        String command = lines[0];
        Map<String,String> headers = new HashMap<String, String>();
        String body = null;
        int i;
        for (i = 1; i < lines.length; i++) {
            if (lines[i].equals("")){
                break;
            }
            String[] header = lines[i].split(":");
            headers.put(header[0],header[1]);
        }
        for (int j = i+1; j < lines.length; j++) {
            body += lines[j];
            if (j!=lines.length-1)
                body += "\n";
        }
        return new Frame(command,headers,body);
    }

   public String toString (){
        String frameString = command + "\n";
        for (Map.Entry<String, String> entry : headers.entrySet()) {
            frameString += entry.getKey() + ":" + entry.getValue() + "\n";
        }
        frameString += "\n" + body+ "\n";
        return frameString;
   }
}
