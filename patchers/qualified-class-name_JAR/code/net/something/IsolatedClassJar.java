package net.something;

import com.cycling74.max.MaxObject;
import com.cycling74.max.Atom;

public class IsolatedClassJar extends MaxObject {
    IsolatedClassJar(Atom[] args) { }

    public void bang() {
        outlet(0, 1);
    }
}
