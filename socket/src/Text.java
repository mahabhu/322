import java.sql.Time;

public class Text extends Message{
    private boolean read;

    public Text(){
        read = false;
    }

    public boolean isRead() {
        return read;
    }

    public void setRead(boolean read) {
        this.read = read;
    }


}
