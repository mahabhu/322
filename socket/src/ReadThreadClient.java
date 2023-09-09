

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Scanner;

public class ReadThreadClient implements Runnable {
    private Thread thr;
    private NetworkUtil networkUtil;
    private ArrayList<Text> unseen;

    public ReadThreadClient(NetworkUtil networkUtil) {
        this.networkUtil = networkUtil;
        this.thr = new Thread(this);
        thr.start();
        unseen = new ArrayList<>();
        File baseDir = new File("");
    }

    void receiveFile(Request pack) throws IOException, ClassNotFoundException {
        System.out.println("Receiving new file.");
        File baseDir = new File("");
        String srcPath = baseDir.getAbsolutePath();
        srcPath = srcPath.concat("\\clients\\"+pack.getFrom()+"\\"+pack.getFileName());
        FileOutputStream fos = new FileOutputStream(srcPath);
        long fileSize = pack.size;
        while(fileSize>0){
            Object o = networkUtil.read();
            if(o instanceof byte[]){
                byte[] packets = (byte[]) o;
                fos.write(packets,0,packets.length);
                fileSize-= packets.length;
            } //ACK

        }
        fos.close();
        System.out.println("yay");
        //networkUtil.write("complete");
        Object o = networkUtil.read();
        if(o instanceof String) {
            String msg = (String) o;
            if (msg.equalsIgnoreCase("complete")) { System.out.println("File receiving complete."); }
            else{ System.out.println("File receiving failed."); }
        }
    }

    public void run() {
        try {
            while (true) {
                Object o = networkUtil.read();
                if (o != null) {
                    if (o instanceof String){
                        if(((String) o).equalsIgnoreCase("duplicate id, invalid")){
                            System.out.println("invalid, closing connection");
                            networkUtil.closeConnection();
                        }
                        else if(((String) o).equalsIgnoreCase("ACK")){
                            networkUtil.setAck(true);
                        }
                        else{
                            System.out.println((String) o);
                        }
                    }
                    else if (o instanceof Boolean){
                        networkUtil.setAck(true);
                    }
                    else if (o instanceof Information){
                        Information info = (Information) o;
                        if(info.getMessage().equalsIgnoreCase("Client list")) {
                            info.printClientList();
                        }
                        else if(info.getMessage().equalsIgnoreCase("File list")) {
                            info.printFileList();
                        }
                        else if(info.getMessage().equalsIgnoreCase("Unread messages")) {
                            info.printMessages();
                        }
                    }
                    else if(o instanceof Text){
                        Text txt = (Text) o;

                    }
                    else if(o instanceof Request){
                        Request req = (Request) o;
                        receiveFile(req);
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


