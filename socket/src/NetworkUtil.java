

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.Socket;

public class NetworkUtil implements Serializable{
    private Socket socket;
    private ObjectOutputStream oos;
    private ObjectInputStream ois;
    public  String lolName;
    public boolean isClose;
    public boolean isAck;

    public NetworkUtil(String s, int port) throws IOException {
        this.socket = new Socket(s, port);
        socket.setSoTimeout(30000);
        oos = new ObjectOutputStream(socket.getOutputStream());
        ois = new ObjectInputStream(socket.getInputStream());
        isClose = false;
    }

    public NetworkUtil(Socket s) throws IOException {
        this.socket = s;
        socket.setSoTimeout(30000);
        oos = new ObjectOutputStream(socket.getOutputStream());
        ois = new ObjectInputStream(socket.getInputStream());
    }

    public Object read() throws IOException, ClassNotFoundException {
        return ois.readUnshared();
    }

    public void write(Object o) throws IOException {
        oos.writeUnshared(o);
    }

    public void closeConnection() throws IOException {
        ois.close();
        oos.close();
        isClose = true;
    }

    public boolean isAck() {
        return isAck;
    }

    public void setAck(boolean ack) {
        //System.out.println(isAck);
        isAck = ack;
    }
}
