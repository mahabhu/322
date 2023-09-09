
import java.io.*;
import java.lang.reflect.Array;
import java.net.SocketTimeoutException;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class ReadThreadServer implements Runnable {
    long MAX_BUFFER_SIZE = 100024 * 1024; // byte
    int MIN_CHUNK_SIZE = 10; // kb
    int MAX_CHUNK_SIZE = 50; // kb
    int BUFFER_FILL = 0;

    private Thread thr;
    private NetworkUtil networkUtil;
    public HashMap<String, NetworkUtil> clientMap;
    public ArrayList<Text> messages;
    public ArrayList<Request> requests;

    //public ArrayList<String> clientList;


    public ReadThreadServer(HashMap<String, NetworkUtil> map, ArrayList<Text> messages, ArrayList<Request> requests, NetworkUtil networkUtil) {
        this.clientMap = map;
        this.messages = messages;
        this.requests = requests;
        this.networkUtil = networkUtil;
        //this.clientList = clientList;
        this.thr = new Thread(this);
        thr.start();
    }

    void receiveFile(Request pack) throws IOException, ClassNotFoundException {
        System.out.println("Receiving new file.");
        File baseDir = new File("");
        String srcPath = baseDir.getAbsolutePath();
        if(pack.isPriv()) srcPath = srcPath.concat("\\files\\"+pack.getFrom()+"\\private\\"+pack.getFileName());
        else srcPath = srcPath.concat("\\files\\"+pack.getFrom()+"\\public\\"+pack.getFileName());
        FileOutputStream fos = new FileOutputStream(srcPath);
        long fileSize = pack.size;
        int CURR_BUFFER_SIZE = BUFFER_FILL;
        boolean incomplete = false;
        while(fileSize>0 && BUFFER_FILL<MAX_BUFFER_SIZE){ // try {// ACK
            Object o = networkUtil.read();

            if(o instanceof byte[]){
                byte[] packets = (byte[]) o;
                fos.write(packets,0,packets.length);
                fileSize-= packets.length;
                BUFFER_FILL+=packets.length;
                if(BUFFER_FILL>MAX_BUFFER_SIZE){
                    incomplete = true;
                }
                NetworkUtil nu = clientMap.get(pack.getFrom());
                if(nu!=null){
                    nu.write("ACK");
                }
            }
        }
        fos.close();
        NetworkUtil nu = clientMap.get(pack.getFrom());
        if(incomplete){
            nu.write("incomplete");
            BUFFER_FILL = CURR_BUFFER_SIZE;
        }
        System.out.println("yay");
        //networkUtil.write("complete");
        Object o = networkUtil.read();
        if(o instanceof String) {
            String msg = (String) o;
            if (msg.equalsIgnoreCase("complete")) { System.out.println("File receiving complete."); }
        }
        else{ System.out.println("File receiving failed."); }
    }

    public void sendFile(String fileName, int CHUNK_SIZE, String from, String to, boolean priv) throws IOException, ClassNotFoundException {

        File baseDir = new File("");
        String srcPath = baseDir.getAbsolutePath();
        if(priv) srcPath = srcPath.concat("\\files\\"+from+"\\private\\"+fileName);
        else srcPath = srcPath.concat("\\files\\"+from+"\\public\\"+fileName);

        File file = new File(srcPath);

        long fileSize = file.length();

        Request initReq = new Request();
        initReq.CHUNK_SIZE = CHUNK_SIZE;
        initReq.setFileName(fileName);
        initReq.setFrom(to);
        initReq.size = fileSize;
        initReq.setMessage("Upload");

        NetworkUtil nu = clientMap.get(to);
        if (nu != null) { nu.write(initReq); } //initiate file reception
        System.out.println("Sending file.");

        FileInputStream fis = new FileInputStream(file);
        int bytes = 0;
        int r = 0;
        try {
            byte[] buffer = new byte[CHUNK_SIZE];
            while ((bytes = fis.read(buffer)) != -1) {
                r += bytes;
                byte[] okay = new byte[bytes];
                for (int i = 0; i < bytes; i++) {
                    okay[i] = buffer[i];
                }
                networkUtil.write(okay);
            }
        }
        catch(SocketTimeoutException e){
            System.out.println("taking too long");
            networkUtil.closeConnection();
            networkUtil.isClose = true;
        }
        fis.close();
        networkUtil.write("complete");

        System.out.println("Sending Complete");
    }

    public void run() {
        try {
            while (true) {
                Object o = networkUtil.read();
                if (o != null) {
                    if (o instanceof Information) {
                        Information info = (Information) o;
                        if(info.getMessage().equalsIgnoreCase("Client list")){//Look Up Client List
                            String to = info.getFrom();
                            ArrayList<String> clientList = new ArrayList<>();
                            ArrayList<Boolean> clientHas = new ArrayList<>();
                            for(String s:clientMap.keySet()){
                                clientList.add(s);
                                clientHas.add(clientMap.get(s).isClose);
                            }
                            info.setClientList(clientList);
                            info.setClientHas(clientHas);
                            NetworkUtil nu = clientMap.get(to);
                            if (nu != null) {
                                nu.write(info);
                            }
                        }
                        else if(info.getMessage().equalsIgnoreCase("File list")){//Look Up Client List
                            String from = info.getFrom();
                            String to = info.getTo();
                            File baseDir = new File("");
                            String logPath = baseDir.getAbsolutePath();
                            if(from.equalsIgnoreCase(to) && info.privs){
                                logPath = logPath.concat("\\files\\"+ from + "\\private");
                            }
                            else{
                                logPath = logPath.concat("\\files\\"+ from + "\\public");
                            }

                            File directory = new File(logPath);
                            String[] list = directory.list();
                            ArrayList<String> fileList = new ArrayList<>();
                            for(String a:list){
                                fileList.add(a);
                            }
                            info.setFileList(fileList);
                            NetworkUtil nu = clientMap.get(to);
                            if (nu != null) {
                                nu.write(info);
                            }
                        }
                    }
                    else if(o instanceof Request){
                        Request req = (Request) o;
                        if(req.getMessage().equalsIgnoreCase("Request")) {
                            for(String a:clientMap.keySet()){
                                if(a.equalsIgnoreCase(req.getFrom())) continue;
                                NetworkUtil nu = clientMap.get(a);
                                if(nu!=null){
                                    nu.write("[server]: File "+req.getFileName()+" has been requested by "+req.getFrom());
                                }
                            }
                            requests.add(req);
                        }
                        else if(req.getMessage().equalsIgnoreCase("Upload")) {
                            for(Request a:requests){
                                if(a.getFileName().equalsIgnoreCase(req.getFileName())){
                                    req.setPriv(false);
                                    NetworkUtil nu = clientMap.get(a.getFrom());
                                    if(nu!=null){
                                        Text txt = new Text();
                                        txt.setFrom("server");
                                        txt.setTo(a.getFrom());
                                        txt.setMessage("File "+a.getFileName()+" is now available!!");
                                        messages.add(txt);
                                    }
                                }
                            }
                            receiveFile(req);
                        }
                        else if(req.getMessage().equalsIgnoreCase("Download")) {
                            sendFile(req.getFileName(),MIN_CHUNK_SIZE,req.getFrom(),req.getTo(),req.isPriv());
                        }
                    }
                    else if(o instanceof Text){
                        Text txt = (Text) o;
                        if(txt.getFrom().equalsIgnoreCase(txt.getTo()) && txt.getMessage().equalsIgnoreCase("+++")){
                            System.out.println("oh noooo");
                            Information info = new Information();
                            ArrayList<Text> msg = new ArrayList<>();
                            System.out.println(messages.size());
                            for(Text t:messages){

                                if(t.getTo().equalsIgnoreCase(txt.getTo()) && !t.isRead()){
                                    msg.add(t);
                                    System.out.println("so so so unusual");
                                    t.setRead(true);
                                }
                            }
                            info.setMessages(msg);
                            info.setMessage("Unread messages");
                            NetworkUtil nu = clientMap.get(txt.getFrom());
                            if (nu != null) {
                                nu.write(info);
                            }
                        }
                        else{
                            System.out.println("working");
                            messages.add(txt);
                        }
                    }
                }
            }
        } catch (Exception e) {
            System.out.println(e);
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}


