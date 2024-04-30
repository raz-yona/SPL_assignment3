package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        
        // you can use any server... 
        String type = args[1];

        if (type.equals("tpc")){
            Server.threadPerClient( // baseServer
            7777, // default port
            () -> new StompProtocolIMP(), //protocol factory
            StompEncDec::new //message encoder decoder factory
            ).serve();
        } 
        else if (type.equals("reactor")){
            Server.reactor(
            Runtime.getRuntime().availableProcessors(),
            7777, // default port
            () -> new StompProtocolIMP(), //protocol factory  
            StompEncDec::new //message encoder decoder factory
            ).serve();
        }
    }
}


    

