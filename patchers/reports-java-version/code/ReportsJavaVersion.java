import com.cycling74.max.MaxObject;
import com.cycling74.max.Atom;

public class ReportsJavaVersion extends MaxObject {
    ReportsJavaVersion(Atom[] args) { }

    public void bang() {
        String version = System.getProperty("java.version");
        outlet(0, version);
    }
}
