package net.something;

import com.cycling74.max.MaxObject;
import com.cycling74.max.Atom;

public class IsolatedClass extends MaxObject {
    IsolatedClass(Atom[] args) { }

    public void bang() {
        outlet(0, 1);
    }
}
