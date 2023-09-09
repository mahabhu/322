

import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Server {
    long MAX_BUFFER_SIZE = 100000 * 1024; // byte
    int MIN_CHUNK_SIZE = 10; // kb
    int MAX_CHUNK_SIZE = 50; // kb

    private ServerSocket serverSocket;
    public HashMap<String, NetworkUtil> clientMap;
    public ArrayList<Text> messages;
    public ArrayList<Request> requests;
    //public ArrayList<String> clientList;

    Server() {
        clientMap = new HashMap<>();
        messages = new ArrayList<>();
        requests = new ArrayList<>();
        //clientList = new ArrayList<>();
        try {
            serverSocket = new ServerSocket(33333);
            while (true) {
                Socket clientSocket = serverSocket.accept();
                serve(clientSocket);
            }
        } catch (Exception e) {
            System.out.println("Server starts:" + e);
        }
    }

    public void serve(Socket clientSocket) throws IOException, ClassNotFoundException {
        NetworkUtil networkUtil = new NetworkUtil(clientSocket);
        String clientName = (String) networkUtil.read();
        if(clientMap.containsKey(clientName) && !clientMap.get(clientName).isClose){
            System.out.println("duplicate");
            networkUtil.write("duplicate id, invalid");
        }
        else {
            clientMap.put(clientName, networkUtil);

            File baseDir = new File("");
            String logPath = baseDir.getAbsolutePath();
            logPath = logPath.concat("\\files\\"+ clientName);
            File directory = new File(logPath);
            System.out.println(directory.getAbsolutePath());
            if(!directory.exists()){
                directory.mkdir();
            }else{
                System.out.println("already exists");
            }
            String gg = logPath.concat("\\public");
            directory = new File(gg);
            if(!directory.exists()){
                directory.mkdir();
            }
            gg = logPath.concat("\\private");
            directory = new File(gg);
            if(!directory.exists()){
                directory.mkdir();
            }
            new ReadThreadServer(clientMap, messages, requests, networkUtil);
        }
    }

    public static void main(String args[]) {
        File baseDir = new File("");
        String logPath = baseDir.getAbsolutePath();
        logPath = logPath.concat("\\files"); //clients local root
        File directory = new File(logPath);
        System.out.println(directory.getAbsolutePath());
        if(!directory.exists()){
            //directory.mkdir();
        }
        Server server = new Server();
    }
}