package bgu.spl.net.srv;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import bgu.spl.net.impl.stomp.Frame;
import bgu.spl.net.impl.stomp.User;

public class ConnectionsIMP<T> implements Connections<T> {

    protected Map<Integer, ConnectionHandler<T>> connections = new HashMap<Integer, ConnectionHandler<T>>(); // map of connection id to connection handler
    protected volatile Map<String, List<Integer>> channels = new HashMap<String, List<Integer>>(); // map of channel name to list of connection ids
    protected Map<Integer, User> users = new HashMap<Integer, User>(); // map of connection id to user
    protected int connectionId = 0; // the next connection id to be assigned
    protected int messageId = 0; // the next message id to be assigned

    public User getUser(int connectionId) {
        return users.get(connectionId);
    }

    public boolean isTopicExist (String topic) {
        return channels.containsKey(topic);
    }

    public void addTopic (String topic) {
        channels.put(topic, new LinkedList<Integer>());
    }

    public void addSubscriber (String topic, int connectionId) {
        channels.get(topic).add(connectionId);
    }

    public String removeSubscriber (int connectionId){
        for (String topic : channels.keySet()) {
            if (channels.get(topic).contains((Integer) connectionId)){
                channels.get(topic).remove((Integer) connectionId);
                return topic;
            }
        }
        return null;
    }


    public int addConnection( ConnectionHandler<T> connectionHandler) { //works only if a client connects to the server
        connections.put(connectionId, connectionHandler);
        connectionId++;
        return (connectionId-1);
    }


    public User getUser (String name) {
        for (User user : users.values()) {
            if (user.getUsername().equals(name)) {
                return user;
            }
        }
        return null;
    }

    public boolean exists(String name) {
        for (User user : users.values()) {
            if (user.getUsername().equals(name)) {
                return true;
            }
        }
        return false;
    }


    public void removeUser(int connectionId) {
        connections.remove(connectionId);
    }

    public Boolean isLoggedIn(int connectionId) {
        return users.get(connectionId).isLoggedIn();
    }

    public void addUser (User user){
        users.put(user.getConnectionId(), user);
        
    }

    public void updateUser (User user, int connectionIdToChange) { 
        users.remove(user.getConnectionId());    
        Map<String, Integer> subscriptions = user.getSubscriptions();
        for (String topic : subscriptions.keySet()) {  // change the connection id in the topics map
            channels.get(topic).remove((Integer) user.getConnectionId());
            channels.get(topic).add(connectionIdToChange);
        }
        user.setConnectionId(connectionIdToChange);
        users.put(connectionIdToChange, user);


    }

    public boolean isSubscribed (int connectionId, String channelName) {
        if(!isTopicExist(channelName))
            addTopic(channelName);
        for (Integer id : channels.get(channelName)) {
            if (id == connectionId) {
                return true;
            }
        }
        return false;
    }



    public void sendError (int connectionId, String message, Frame frame) { //need to add a disconnect in every error
        String receiptId="";
        Map<String, String> thisHeaders=frame.getHeaders();
        Map<String, String> headers = new HashMap<String, String>();
        if (thisHeaders.containsKey("receipt-id")) {
            receiptId=thisHeaders.get("receipt-id");
            headers.put("receipt-id", receiptId);
        }
        headers.put("message", message);
        Frame error = new Frame("ERROR", headers, "");
        send(connectionId,  (T) error.toString());
        disconnect(connectionId);
    }


    public void sendConnected (int connectionId){
        Map<String, String> headers = new HashMap<String, String>();
        headers.put("version", "1.2");
        Frame connected = new Frame("CONNECTED", headers, "");
        send(connectionId,  (T) connected.toString());
    }


    public void sendMessage (int connectionId, String subscriptionId, String messageId, String body, String topic){
        Map<String, String> headers = new HashMap<String, String>();
        headers.put("subscription", subscriptionId);
        headers.put("message-id", messageId);
        headers.put("destination", topic);
        Frame message = new Frame("MESSAGE", headers, body);
        send(connectionId,  (T) message.toString());       
    }

    public void sendReceipt (String receiptId, int connectionId){
        Map<String, String> headers = new HashMap<String, String>();
        headers.put("receipt-id", receiptId);
        Frame receipt = new Frame("RECEIPT", headers, "");
        send(connectionId,  (T) receipt.toString());       
    }


    @Override
    public boolean send(int connectionId, T msg) {
        // TODO Auto-generated method stub
        //we use try cath because if connectionHandler.sent will throw an exception we want to return false
        boolean isSent = false;
        try{   
            if (connections.get(connectionId) != null){
                connections.get(connectionId).send(msg);
                isSent = true;
            }
        }
        catch (Exception e) {
            isSent = false;
        }
        return isSent;
    }

    @Override
    public void send(String channel, T msg) {     //we get from msg the body of SEND Frame
        // TODO Auto-generated method stub
        for (int connectionId : channels.get(channel)) { //we need to meka a message frame and send it to all the users in the channel
            int intSubscriptionId = users.get(connectionId).getSubscriptionId(channel);
            String subscriptionId = Integer.toString(intSubscriptionId);
            String messageId = Integer.toString(this.messageId);
            sendMessage(connectionId, subscriptionId, messageId, (String) msg, channel);  
        }
        this.messageId++;
        
    }

    public void disconnect (int connectionId, String receiptId) {
        disconnect(connectionId);
        sendReceipt(receiptId, connectionId);
    }

    @Override
    public void 
    disconnect(int connectionId) {
        // TODO Auto-generated method stub
        User user = getUser(connectionId);
        if (user != null) {
            user.setLogged(false);
            removeUser(connectionId);  //removes user from connections map
            user.removeAllSubscriptions();  //remove all subscriptions from user
        }
        for (String topic : channels.keySet()) {  //remove user from all channels
            if (channels.get(topic).contains((Integer) connectionId)){
                channels.get(topic).remove((Integer) connectionId);
            }
        }
        ConnectionHandler<T> connectionHandler = connections.get(connectionId);
        try {
            if (connectionHandler != null)
            connectionHandler.close();
        } catch (IOException e) {}
    }
}
