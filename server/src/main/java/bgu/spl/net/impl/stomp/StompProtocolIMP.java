package bgu.spl.net.impl.stomp;

import java.util.Map;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsIMP;

public class StompProtocolIMP<T> implements StompMessagingProtocol<T> {
    ConnectionsIMP<String> connections;
    int connectionId;
    boolean shouldTerminate=false;
    

    @Override
    public void start(int connectionId, Connections<T> connections) {
        this.connections=(ConnectionsIMP<String>) connections;
        this.connectionId=connectionId;
        
    }

    @Override
    public void process(T message) {
        Frame frame = Frame.stringToFrame((String)message);
        String command = frame.getCommand();
        if(command.equals("CONNECT")){
            connectResponse(frame);
        }
        else if(command.equals("SEND"))
        sendResponse(frame);
        else if(command.equals("SUBSCRIBE"))
        subscribeResponse(frame);
        else if(command.equals("UNSUBSCRIBE"))
        unsubscribeResponse(frame);
        else if(command.equals("DISCONNECT"))
        disconnectResponse(frame);
    }

    @Override
    public boolean shouldTerminate() { 
        // TODO Auto-generated method stub
        return shouldTerminate;
    }

    public void connectResponse(Frame frame){
        String name = frame.getHeaders().get("login");
        String password=frame.getHeaders().get("passcode");
        String id=null;
        if (frame.getHeaders().get("id")!=null){
             id=frame.getHeaders().get("id");
        }
        //User user = connections.isExist(name);
        if(!connections.exists(name)){
            newUser(frame);
            connections.sendConnected(connectionId);
            //Probably need to add
            // if (id!=null){
            //     connections.sendReceipt(id, connectionId);
            // }
        }
        else{
            User user = connections.getUser(name);
            if (user.isLoggedIn()){
                connections.sendError(connectionId, "User already logged in", frame);
            }
            else{
                if (user.getPassword().equals(password)){  //user exists and not logged in
                    connections.updateUser(user, connectionId);    
                    connections.sendConnected(connectionId);
                    if (id!=null){
    
                        connections.sendReceipt(id, connectionId);
                    }
                }
                else{
                    connections.sendError(connectionId, "Wrong password", frame);
                }
            }
        }
    }

    public void sendResponse(Frame frame){
        Map<String, String> headers = frame.getHeaders();
        String destination = headers.get("destination");
        String body = frame.getBody();
        if (connections.isSubscribed(connectionId, destination)){
            connections.send(destination, body);
        }
        else{
            connections.sendError(connectionId, "User is not subscribed to this channel", frame);
        }
        
        

    }

    public void subscribeResponse(Frame frame){
        Map<String, String> headers = frame.getHeaders();
        String destination = headers.get("destination");
        String id = headers.get("id");
        int subscriptionId = Integer.parseInt(id);
        if (connections.isSubscribed(connectionId, destination)){ // opens a new channel if it does'nt exist
            connections.sendError(connectionId, "User is already subscribed to this channel", frame);
        }
        
        else{
            connections.addSubscriber(destination, connectionId);
            User user= connections.getUser(connectionId);
            user.addSubscription(destination, subscriptionId);
            String receiptId = headers.get("receipt");
            connections.sendReceipt(receiptId, connectionId);
        }
    }

    public void unsubscribeResponse(Frame frame){
        Map<String, String> headers= frame.getHeaders();
        String id = headers.get("id");
        //int subscriptionId = Integer.parseInt(id);
        User user = connections.getUser(connectionId);
        String destinatination=connections.removeSubscriber(connectionId);
        if (destinatination==null){
            connections.sendError(connectionId, "User is not subscribed to this channel", frame);
        }
        else{
            user.removeSubscription(destinatination);
            String receiptId = headers.get("receipt");
            connections.sendReceipt(receiptId, connectionId);
        }
    }

    public void disconnectResponse(Frame frame){
        shouldTerminate=true;
        Map<String, String> headers= frame.getHeaders();
        String receipt = headers.get("receipt");
        Integer receiptId = Integer.parseInt(receipt);
        User user = connections.getUser(connectionId);
        if (!user.isContainReceipt(receiptId)){
            connections.sendError(connectionId, "wrong receipt-id", frame);
        }
        else{
            connections.disconnect(connectionId, receipt);
        }
    }

    public void newUser (Frame frame){

        User user = new User(frame.getHeaders().get("login"), frame.getHeaders().get("passcode"), connectionId);
        connections.addUser(user);
        user.setLogged(true);

        
    }

    

}
