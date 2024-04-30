package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class User {

    private String username;
    private String password;
    private boolean loggedIn;
    private int connectionId;
    private Map<String,Integer>subscriptions;  //a map of the connection between the subscription id and the channel name

    public User(String username, String password, int connectionId) {
        this.username = username;
        this.password = password;
        this.loggedIn = false;
        this.connectionId = connectionId;
        subscriptions = new HashMap<>();
        
        
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public boolean isLoggedIn() {
        return loggedIn;
    }

    public int getConnectionId() {
        return connectionId;
    }

    public int getSubscriptionId (String channelName) {
        return subscriptions.get(channelName);
    }

    public Map<String, Integer> getSubscriptions() {
        return subscriptions;
    }

    public void setLogged (boolean logged) {
        this.loggedIn = logged;
    }

    public void setConnectionId(int connectionId) {
        this.connectionId = connectionId;
    }

    public void addSubscription (String channelName, int subscriptionId) {
        subscriptions.put(channelName, subscriptionId);
    }

    public void removeSubscription (String channelName) {
        subscriptions.remove(channelName);
    }

    public void removeAllSubscriptions () {
        subscriptions.clear();
    }

    public boolean isContainReceipt (Integer receiptId) {
        return subscriptions.containsValue(receiptId);
    }







    
}
