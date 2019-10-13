import com.cycling74.max.MaxObject;
import com.cycling74.max.Atom;

import java.lang.ClassLoader;
import java.net.URLClassLoader;
import java.net.URL;

public class TestContextClassLoader extends MaxObject {
    static final URL[] EMPTY_URLs = new URL[]{};

    class XX extends URLClassLoader {
        XX() {
            super(EMPTY_URLs, TestContextClassLoader.class.getClassLoader());
        }

        boolean poink() {
            try {
                System.out.println("1: " + TestContextClassLoader.class.getClassLoader().loadClass("java.net.URL"));
                System.out.println("2: " + Thread.currentThread().getContextClassLoader().loadClass("java.net.URL"));
                System.out.println("3: " + loadClass("java.net.URL"));
                return (Thread.currentThread().getContextClassLoader() != null);
            } catch (Exception exn) {
                exn.printStackTrace();
                return false;
            }
        }
    }

    TestContextClassLoader(Atom[] args) { }

    public void bang() {
        XX xx = new XX();
        if (xx.poink()) {
            outlet(0, 1);
        } else {
            outlet(0, 0);
        }
    }
}
