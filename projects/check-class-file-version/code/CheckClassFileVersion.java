import com.cycling74.max.MaxObject;
import com.cycling74.max.Atom;
import com.cycling74.max.MXJClassLoader;;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;


public class CheckClassFileVersion extends MaxObject {
    CheckClassFileVersion(Atom[] args) {
    }

    public void check(String filename) {
        InputStream in = null;
        DataInputStream data = null;

        try {
            in = new FileInputStream(filename);
            data = new DataInputStream(in);

            int magic = data.readInt();
            if (magic != 0XCAFEBABE) {
                error("not a valid class!");
            } else {
                int minor = data.readUnsignedShort();
                int major = data.readUnsignedShort();
                outlet(0, new int[] { major, minor });
            }
        } catch (Exception exn) {
            error(">>> " + exn.getMessage() + " <<<");
        } finally {
            if (data != null) {
                try { data.close(); } catch (Exception exn) {
                    error(exn.getMessage());
                };
            }
            if (in != null) {
                try { in.close(); } catch (Exception exn) {
                    error(exn.getMessage());
                };
            }
        }
    }
}
