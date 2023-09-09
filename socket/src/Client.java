import java.io.File;
import java.io.Serializable;
import java.util.Scanner;

public class Client implements Serializable {

    public Client(String serverAddress, int serverPort) {
        try {
            System.out.print("Enter name of the client: ");
            Scanner scanner = new Scanner(System.in);
            String clientName = scanner.nextLine();//client login

            File baseDir = new File("");
            String logPath = baseDir.getAbsolutePath();

            logPath = logPath.concat("\\clients"); //clients local root
            File directory = new File(logPath);
            System.out.println(directory.getAbsolutePath());
            if(!directory.exists()){
                directory.mkdir();
            }

            logPath = logPath.concat("\\"+ clientName); //clients local folder
            directory = new File(logPath);
            System.out.println(directory.getAbsolutePath());
            if(!directory.exists()){
                directory.mkdir();
            }else{
                //System.out.println("already exists");
            }
            NetworkUtil networkUtil = new NetworkUtil(serverAddress, serverPort);
            networkUtil.write(clientName);
            new ReadThreadClient(networkUtil);
            new WriteThreadClient(networkUtil, clientName);
        } catch (Exception e) {
            System.out.println(e);
        }
    }

    public static void main(String args[]) {
        String serverAddress = "127.0.0.1";
        int serverPort = 33333;
        Client client = new Client(serverAddress, serverPort);
    }
}

