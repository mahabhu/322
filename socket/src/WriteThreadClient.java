import java.io.*;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Scanner;

class WriteThreadClient implements Runnable {

    long MAX_BUFFER_SIZE = 100000 * 1024; // byte
    int MIN_CHUNK_SIZE = 10; // kb
    int MAX_CHUNK_SIZE = 50; // kb

    private Thread thr;
    private NetworkUtil networkUtil;
    String name;
    private boolean isLive;

    public WriteThreadClient(NetworkUtil networkUtil, String name) {
        this.networkUtil = networkUtil;
        this.name = name;
        isLive = true;
        this.thr = new Thread(this);
        thr.start();
    }

    public void showFiles(String path){
        File baseDir = new File("");
        String logPath = baseDir.getAbsolutePath();
        logPath = logPath.concat("\\"+path);
        File directory = new File(logPath);
        String[] list = directory.list();
        for(String a:list){
            System.out.println(a);
        }
    }

    public void sendFile(String fileName, int CHUNK_SIZE, String name, boolean priv) throws IOException, ClassNotFoundException {

        File baseDir = new File("");
        String srcPath = baseDir.getAbsolutePath();
        srcPath = srcPath.concat("\\clients\\"+name+"\\"+fileName);
        File file = new File(srcPath);

        long fileSize = file.length();

        Request initReq = new Request();
        initReq.CHUNK_SIZE = CHUNK_SIZE;
        initReq.setPriv(priv);
        initReq.setFileName(fileName);
        initReq.setFrom(name);
        initReq.size = fileSize;
        initReq.setMessage("Upload");

        networkUtil.write(initReq); //Sends initial sending command
        System.out.println("Sending file.");

        FileInputStream fis = new FileInputStream(file);
        int bytes = 0;
        int r = 0;
        byte[] buffer = new byte[CHUNK_SIZE];
        try {
            while ((bytes = fis.read(buffer)) != -1) {
                r += bytes;
                byte[] okay = new byte[bytes];
                for (int i = 0; i < bytes; i++) {
                    okay[i] = buffer[i];
                }
                networkUtil.setAck(false);
                networkUtil.write(okay);
                while(!networkUtil.isAck()){
                    System.out.print("");
                }
            }
        }
        catch(SocketTimeoutException timeout){
            System.out.println("taking too long");
            networkUtil.closeConnection();
            networkUtil.isClose = true;
        }
        fis.close();
        networkUtil.write("complete");
        //complete
        System.out.println("Sending Complete");
    }

    public void option(String type) throws IOException, ClassNotFoundException {
        Scanner input = new Scanner(System.in);
        if(type.equalsIgnoreCase("1")){//Look up client list
            Information info = new Information();
            info.setFrom(name);
            info.setMessage("Client list");
            networkUtil.write(info);
        }
        else if(type.equalsIgnoreCase("2")){//Create a file
            System.out.println("Enter file name:");
            String fileName = input.nextLine();
            File baseDir = new File("");
            String srcPath = baseDir.getAbsolutePath();
            srcPath = srcPath.concat("\\clients\\"+name+"\\"+fileName);
            FileOutputStream fos = new FileOutputStream(srcPath);
            fos.close();
            System.out.println(fileName+" created");
        }
        else if(type.equalsIgnoreCase("3")){//Show my files
            Information info = new Information();
            System.out.println("Private or public?(1/2)");
            String priv = input.nextLine();
            boolean privs = false;
            if(priv.equalsIgnoreCase("1")) privs = true;
            info.privs = privs;
            info.setFrom(name);
            info.setTo(name);
            info.setMessage("File list");
            networkUtil.write(info);
        }
        else if(type.equalsIgnoreCase("4")){//Show others files
            Information info = new Information();
            System.out.println("Enter source client:");
            String src = input.nextLine();
            info.setFrom(src);
            info.setTo(name);
            info.setMessage("File list");
            networkUtil.write(info);
        }
        else if(type.equalsIgnoreCase("5")){//Send message
            System.out.println("Enter Recipient:");
            String to = input.nextLine();
            System.out.println("Enter message:");
            String msg = input.nextLine();
            Text txt = new Text();
            txt.setFrom(name);
            txt.setTo(to);
            txt.setMessage(msg);
            networkUtil.write(txt);
        }
        else if(type.equalsIgnoreCase("6")){//Send file request
            System.out.println("Enter file name");
            String fileName = input.nextLine();
            Request req = new Request();
            req.setFrom(name);
            req.setMessage("Request");
            req.setFileName(fileName);
            networkUtil.write(req);
        }
        else if(type.equalsIgnoreCase("7")){//Upload file
            System.out.println("Choose a file to upload:");
            showFiles("clients\\"+name);
            String fileName = input.nextLine();
            System.out.println("Private or public?(1/2)");
            String priv = input.nextLine();
            boolean privs = false;
            if(priv.equalsIgnoreCase("1")){
                privs = true;
            }
            int chunkSize = MIN_CHUNK_SIZE + (int)((MAX_CHUNK_SIZE-MIN_CHUNK_SIZE)*Math.random());
            //System.out.println(chunkSize);
            sendFile(fileName,chunkSize,name,privs);
        }
        else if(type.equalsIgnoreCase("8")){//Download file
            System.out.println("Select a client:");
            option("1");
            String from = input.nextLine();

            boolean privs = false;

            if(from.equalsIgnoreCase(name)) {
                System.out.println("Private or public?(1/2)");
                String priv = input.nextLine();

                if (priv.equalsIgnoreCase("1")) privs = true;
            }


            System.out.println("Choose a file:");
            Information info = new Information();
            info.privs = privs;
            info.setFrom(from);
            info.setTo(name);
            info.setMessage("File list");
            networkUtil.write(info);

            String fileName = input.nextLine();
            Request request = new Request();
            request.setMessage("Download");
            request.setFrom(from);
            request.setTo(name);
            request.setPriv(privs);
            request.setFileName(fileName);
            networkUtil.write(request);
        }
        else if(type.equalsIgnoreCase("9")){//Close Connection
            System.out.println("Closing connection");
            isLive = false;
        }
        else if(type.equalsIgnoreCase("10")){//See unread messages
            Text txt = new Text();
            txt.setFrom(name);
            txt.setTo(name);
            txt.setMessage("+++");
            networkUtil.write(txt);
        }
    }

    public void run() { //client console
        try {
            Scanner input = new Scanner(System.in);

            while (true && isLive) {
                System.out.println("\nEnter Command");
                System.out.println("1. Look up client list    2. Make a file      3. Show my files");
                System.out.println("4. Show others files      5. Send message     6. Send file request");
                System.out.println("7. Upload file            8. Download file    9. Close connection");
                System.out.println("10.See unread messages ");
                String type = input.nextLine();
                option(type);
                Thread.sleep(500);
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


