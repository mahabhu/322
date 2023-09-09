import java.util.ArrayList;

public class Information extends Message{
    private ArrayList<String> clientList;
    private ArrayList<Boolean> clientHas;
    private ArrayList<String> fileList;
    private ArrayList<Text> messages;
    public boolean privs;

    public ArrayList<String> getFileList() {
        return fileList;
    }

    public void setFileList(ArrayList<String> fileList) {
        this.fileList = fileList;
    }

    public ArrayList<String> getClientList() {
        return clientList;
    }

    public void setClientHas(ArrayList<Boolean> clientHas) {
        this.clientHas = clientHas;
    }

    public ArrayList<Boolean> getClientHas() {
        return clientHas;
    }

    public void setClientList(ArrayList<String> clientList) {
        this.clientList = clientList;
    }

    public ArrayList<Text> getMessages() {
        return messages;
    }

    public void setMessages(ArrayList<Text> messages) {
        this.messages = messages;
    }

    public void printClientList(){
        System.out.println("List of all clients:");
        for(int i=0; i<clientList.size(); i++)
        {
            String s = "Online";
            if(clientHas.get(i)) s = "Offline";
            System.out.println(clientList.get(i) + " : " + s);
        }
    }

    public void printFileList(){
        System.out.println("List of all files of "+ from +":");
        for(int i=0; i<fileList.size(); i++)
        {
            System.out.println(fileList.get(i));
        }
    }

    public void printMessages(){
        for(Text a:messages){
            System.out.println("["+a.getFrom()+"]"+": "+a.getMessage());
        }
        if(messages.size()==0){
            System.out.println("No unseen messages :)");
        }
    }
}

