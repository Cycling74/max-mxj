/*
	Copyright (c) 2012 Cycling '74

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
	and associated documentation files (the "Software"), to deal in the Software without restriction, 
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies 
	or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

import com.cycling74.max.*;

public class Plus2 extends MaxObject {

	private Atom addend = Atom.newAtom(0);
	
	public Plus2(Atom[] args) {
		declareIO(2,1);
		if (args.length > 0) {
			if (args[0].isInt() || args[0].isFloat()) {
				addend = args[0];
			}
		} 
	}
	
	public void inlet(int i) {
		if (getInlet() == 0) {
			if (addend.isInt()) {
				outlet(0, i + addend.getInt());
			} else {
				outlet(0, (float)i + addend.getFloat());
			}
		} else {
			if (addend.isInt()) {
				addend = Atom.newAtom(i);
			} else {
				addend = Atom.newAtom((float)i);
			}
		}
	}
	
	public void inlet(float f) {
		if (getInlet() == 0) {
			if (addend.isInt()) {
				outlet(0, (int)f + addend.getInt());
			} else {
				outlet(0, f + addend.getFloat());
			}
		} else {
			if (addend.isInt()) {
				addend = Atom.newAtom((int)f);
			} else {
				addend = Atom.newAtom(f);
			}
		}
	}
}