package com.cycling74.max;

import java.util.prefs.*;
import java.io.*;
import java.awt.Color;

public class MXJPreferences {

	public static final String MXJPREFFILE = "mxjpreferences.xml";
	// ELEMENT/TAG NAMES
	public static final String MXJ_COMPILER_COMMAND = "COMMAND";
	public static final String MXJ_COMPILER_OPTIONS = "OPTIONS";
	public static final String MXJ_COMPILER_BUILD_ROOT = "BUILD_ROOT";

	// MXJEditor prefs
	public static final String MXJ_EDITOR_FOREGROUND_COLOR = "FOREGROUND_COLOR";
	public static final String MXJ_EDITOR_BACKGROUND_COLOR = "BACKGROUND_COLOR";
	public static final String MXJ_EDITOR_COMMENT1_COLOR = "COMMENT1_COLOR";
	public static final String MXJ_EDITOR_COMMENT2_COLOR = "COMMENT2_COLOR";
	public static final String MXJ_EDITOR_KEYWORD1_COLOR = "KEYWORD1_COLOR";
	public static final String MXJ_EDITOR_KEYWORD2_COLOR = "KEYWORD2_COLOR";
	public static final String MXJ_EDITOR_KEYWORD3_COLOR = "KEYWORD3_COLOR";
	public static final String MXJ_EDITOR_LITERAL1_COLOR = "LITERAL1_COLOR";
	public static final String MXJ_EDITOR_LITERAL2_COLOR = "LITERAL2_COLOR";
	public static final String MXJ_EDITOR_LABEL_COLOR = "LABEL_COLOR";
	public static final String MXJ_EDITOR_OPERATOR_COLOR = "OPERATOR_COLOR";
	public static final String MXJ_EDITOR_INVALID_COLOR = "INVALID_COLOR";
	public static final String MXJ_EDITOR_CARET_COLOR = "CARET_COLOR";
	public static final String MXJ_EDITOR_BLOCK_CARET = "BLOCK_CARET";
	public static final String MXJ_EDITOR_CARET_BLINKS = "CARET_BLINKS";
	public static final String MXJ_EDITOR_EOL_MARKER_COLOR = "EOL_MARKER_COLOR";
	public static final String MXJ_EDITOR_USE_EOL_MARKERS = "USE_EOL_MARKERS";
	public static final String MXJ_EDITOR_SELECTION_COLOR = "SELECTION_COLOR";
	public static final String MXJ_EDITOR_LINE_HILITE = "LINE_HILITE";
	public static final String MXJ_EDITOR_LINE_HILITE_COLOR = "LINE_HILITE_COLOR";
	public static final String MXJ_EDITOR_BRACKET_HILITE = "BRACKET_HILITE";
	public static final String MXJ_EDITOR_BRACKET_HILITE_COLOR = "BRACKET_HILITE_COLOR";

	private static Preferences _prefs = null;
	private static Preferences _mxj_prefs = null;
	private static Preferences _mxj_editor_prefs = null;
	private static Preferences _mxj_compiler_prefs = null;
	private static String _default_compiler = null;
	private static String _default_compiler_build_root = null;

	static {
		// Maybe there are multiple installs?
		String filename = MaxSystem.locateFile(MXJPREFFILE);

		if (filename == null) {
			String prefspath = MaxSystem.getPreferencesPath();
			filename = prefspath + "/" + MXJPREFFILE;
		}

		File f = new File(filename);
		_prefs = Preferences.userRoot();
		_mxj_prefs = _prefs.node("MXJ_PREFS");
		_mxj_editor_prefs = _mxj_prefs.node("MXJ_EDITOR_PREFS");
		_mxj_compiler_prefs = _mxj_prefs.node("MXJ_COMPILER_PREFS");

		try {
			_mxj_editor_prefs.clear();
			_mxj_compiler_prefs.clear();
		} catch (Exception e) {

		}

		_default_compiler = _get_default_compiler();
		_default_compiler_build_root = _get_default_compiler_build_root();
		if (f.exists()) {
			try {
				Preferences.importPreferences(new BufferedInputStream(
						new FileInputStream(f)));
			} catch (IOException e) {
				System.err.println("(mxj) IO Error importing preferences: "
						+ filename);
				e.printStackTrace();
			} catch (InvalidPreferencesFormatException ipe) {
				System.err.println("(mxj) Invalid preference file format: "
						+ filename);
				ipe.printStackTrace();
			}
		} else {
			write_compiler_prefs();
			write_editor_prefs();
			savePrefs();
		}

	}

	// can't instantiate
	private MXJPreferences() {
	}

	public static void writeToDisk() {
		String prefspath = MaxSystem.getPreferencesPath();
		String filename = prefspath + "/" + MXJPREFFILE;
		try {
			_mxj_prefs.exportSubtree(new BufferedOutputStream(
					new FileOutputStream(filename)));
		} catch (Exception e) {
			System.err.println("(mxj) Problem writing preferences to :"
					+ filename);
			e.printStackTrace();
		}
	}

	public static void readFromDisk() {
		String prefspath = MaxSystem.getPreferencesPath();
		String filename = prefspath + "/" + MXJPREFFILE;
		File f = new File(filename);
		if (f.exists()) {
			try {
				Preferences.importPreferences(new BufferedInputStream(
						new FileInputStream(f)));
			} catch (IOException e) {
				System.err.println("(mxj) IO Error importing preferences: "
						+ filename);
				e.printStackTrace();
			} catch (InvalidPreferencesFormatException ipe) {
				System.err.println("(mxj) Invalid preference file format: "
						+ filename);
				ipe.printStackTrace();
			}
		}
		// _prefs = Preferences.userRoot();
	}

	public static void flush() {
		try {
			_mxj_compiler_prefs.flush();
		} catch (Exception e) {
			System.err.println("(mxj) Problem flushing mxj preferences.");
			e.printStackTrace();
		}
	}

	public static void setMXJCompilerCommmand(String command) {
		_mxj_compiler_prefs.put(MXJ_COMPILER_COMMAND, command);
		flush();
		writeToDisk();
	}

	public static void setMXJCompilerBuildRoot(String root) {
		_mxj_compiler_prefs.put(MXJ_COMPILER_BUILD_ROOT, root);
		flush();
		writeToDisk();
	}

	public void setMXJCompilerOptions(String options) {
		_mxj_compiler_prefs.put(MXJ_COMPILER_OPTIONS, options);
		flush();
		writeToDisk();
	}

	public static String[] getMXJCompilerOptions() {
		String ret = _prefs.get(MXJ_COMPILER_OPTIONS, "");
		if (ret.equals(""))
			return null;
		else
			return ret.split(" ");
	}

	private static String _get_default_compiler_build_root() {
		return MaxSystem.maxPathToNativePath("c74:/java-classes/classes");
	}

	private static String _get_default_compiler() {
		String javahome = System.getProperty("java.home");
		StringBuffer ret = new StringBuffer(javahome + "/bin/javac");
		if (MaxSystem.isOsWindows())
			ret.append(".exe");
		return ret.toString();
	}

	public static String getMXJCompilerCommmand() {
		return _mxj_compiler_prefs.get(MXJ_COMPILER_COMMAND, _default_compiler);
	}

	public static String getMXJCompilerBuildRoot() {
		return _mxj_compiler_prefs.get(MXJ_COMPILER_BUILD_ROOT,
				_default_compiler_build_root);
	}

	// /Begin editor stuff
	public static Color getMXJEditorFGColor() {
		int val;
		String c = "000000";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_FOREGROUND_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.black;
	}

	public static void setMXJEditorFGColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_FOREGROUND_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorBGColor() {
		int val;
		// String c = "FFE9FF";
		String c = "FFFFFF";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_BACKGROUND_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0xFFFFFF);
	}

	public static void setMXJEditorBGColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_BACKGROUND_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorComment1Color() {
		int val;
		String c = "FF0000";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_COMMENT1_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0xFF2288);
	}

	public static void setMXJEditorComment1Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_COMMENT1_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorComment2Color() {
		int val;
		String c = "ff0000";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_COMMENT2_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0xff0000);
	}

	public static void setMXJEditorComment2Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_COMMENT2_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorKeyword1Color() {
		int val;
		String c = RGBtoHex(Color.blue.getRGB());
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_KEYWORD1_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.blue;
	}

	public static void setMXJEditorKeyword1Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_KEYWORD1_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorKeyword2Color() {
		int val;
		// String c = "00aa22";
		String c = "339999";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_KEYWORD3_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0x339999);
	}

	public static void setMXJEditorKeyword2Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_KEYWORD3_COLOR, RGBtoHex(c));

	}

	public static Color getMXJEditorKeyword3Color() {
		int val;
		String c = RGBtoHex(Color.magenta.getRGB());
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_KEYWORD2_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.magenta;
	}

	public static void setMXJEditorKeyword3Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_KEYWORD2_COLOR, RGBtoHex(c));

	}

	public static Color getMXJEditorLiteral1Color() {
		int val;
		// String c = "dd0066";
		String c = "888888";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_LITERAL1_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0x888888);
	}

	public static void setMXJEditorLiteral1Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_LITERAL1_COLOR, RGBtoHex(c));

	}

	public static Color getMXJEditorLiteral2Color() {
		int val;
		String c = RGBtoHex(Color.magenta.getRGB());
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_LITERAL2_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.magenta;
	}

	public static void setMXJEditorLiteral2Color(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_LITERAL2_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorLabelColor() {
		int val;
		String c = "0000ff";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_LABEL_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0x0000ff);
	}

	public static void setMXJEditorLabelColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_LABEL_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorOperatorColor() {
		int val;
		String c = RGBtoHex(Color.orange.getRGB());
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_OPERATOR_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.orange;
	}

	public static void setMXJEditorOperatorColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_OPERATOR_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorInvalidColor() {
		int val;
		String c = RGBtoHex(Color.black.getRGB());
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_INVALID_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.black;
	}

	public static void setMXJEditorInvalidColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_INVALID_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorCaretColor() {
		int val;
		String c = "FF0000";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_CARET_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0xFF0000);
	}

	public static void setMXJEditorCaretColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_CARET_COLOR, RGBtoHex(c));
	}

	public static boolean getMXJEditorBlockCaret() {
		boolean val = true;
		String s = _mxj_editor_prefs.get(MXJ_EDITOR_BLOCK_CARET, "true");
		if (s.equals("true"))
			return true;
		else
			return false;
	}

	public static void setMXJEditorBlockCaret(boolean b) {
		String s;
		if (b)
			s = "true";
		else
			s = "false";
		_mxj_editor_prefs.put(MXJ_EDITOR_BLOCK_CARET, s);
	}

	public static boolean getMXJEditorCaretBlinks() {
		boolean val = false;
		String s = _mxj_editor_prefs.get(MXJ_EDITOR_CARET_BLINKS, "false");
		if (s.equals("true"))
			return true;
		else
			return false;
	}

	public static void setMXJEditorCaretBlinks(boolean b) {
		String s;
		if (b)
			s = "true";
		else
			s = "false";
		_mxj_editor_prefs.put(MXJ_EDITOR_CARET_BLINKS, s);
	}

	public static Color getMXJEditorEOLMarkerColor() {
		int val;
		String c = "999966";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_EOL_MARKER_COLOR, c), 16));
		} catch (NumberFormatException nfe) {
			nfe.printStackTrace();
		}
		return new Color(0x999966);
	}

	public static void setMXJEditorEOLMarkerColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_EOL_MARKER_COLOR, RGBtoHex(c));
	}

	public static boolean getMXJEditorUseEOLMarkers() {
		boolean val = true;
		String s = _mxj_editor_prefs.get(MXJ_EDITOR_USE_EOL_MARKERS, "true");
		if (s.equals("true"))
			return true;
		else
			return false;
	}

	public static void setMXJEditorUseEOLMarkers(boolean b) {
		String s;
		if (b)
			s = "true";
		else
			s = "false";
		_mxj_editor_prefs.put(MXJ_EDITOR_USE_EOL_MARKERS, s);
	}

	public static Color getMXJEditorBracketHiliteColor() {
		int val;
		String c = "FF66FF";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_BRACKET_HILITE_COLOR, c),
					16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0xFF66ff);
	}

	public static void setMXJEditorBracketHiliteColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_BRACKET_HILITE_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorLineHiliteColor() {
		int val;
		String c = "e0e0e0";
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_LINE_HILITE_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return new Color(0xe0e0e0);
	}

	public static void setMXJEditorLineHiliteColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_LINE_HILITE_COLOR, RGBtoHex(c));
	}

	public static Color getMXJEditorSelectionColor() {
		int val;
		String c = RGBtoHex(Color.yellow.getRGB());
		try {
			return new Color(Integer.parseInt(
					_mxj_editor_prefs.get(MXJ_EDITOR_SELECTION_COLOR, c), 16));
		} catch (NumberFormatException nfe) {

		}
		return Color.yellow;
	}

	public static void setMXJEditorSelectionColor(int c) {
		_mxj_editor_prefs.put(MXJ_EDITOR_SELECTION_COLOR, RGBtoHex(c));
	}

	public static boolean getMXJEditorLineHilite() {
		boolean val = false;
		String s = _mxj_editor_prefs.get(MXJ_EDITOR_LINE_HILITE, "false");
		if (s.equals("true"))
			return true;
		else
			return false;
	}

	public static void setMXJEditorLineHilite(boolean b) {
		String s;
		if (b)
			s = "true";
		else
			s = "false";
		_mxj_editor_prefs.put(MXJ_EDITOR_LINE_HILITE, s);
	}

	public static boolean getMXJEditorBracketHilite() {
		boolean val = true;
		String s = _mxj_editor_prefs.get(MXJ_EDITOR_BRACKET_HILITE, "true");
		if (s.equals("true"))
			return true;
		else
			return false;
	}

	public static void setMXJEditorBracketHilite(boolean b) {
		String s;
		if (b)
			s = "true";
		else
			s = "false";
		_mxj_editor_prefs.put(MXJ_EDITOR_BRACKET_HILITE, s);
	}

	// /////////////////end editor prefs

	public static void savePrefs() {
		flush();
		writeToDisk();
	}

	private static String RGBtoHex(int RGBcolor) {
		Color color = new Color(RGBcolor);
		StringBuffer webcolor = new StringBuffer();
		int r = color.getRed();
		int g = color.getGreen();
		int b = color.getBlue();

		// webcolor.append("0x");
		if (r < 16)
			webcolor.append("0");
		webcolor.append(Integer.toHexString(r));
		if (g < 16)
			webcolor.append("0");
		webcolor.append(Integer.toHexString(g));
		if (b < 16)
			webcolor.append("0");
		webcolor.append(Integer.toHexString(b));

		return webcolor.toString();
	}

	private static void write_compiler_prefs() {
		setMXJCompilerCommmand(getMXJCompilerCommmand());
		setMXJCompilerBuildRoot(getMXJCompilerBuildRoot());
	}

	private static void write_editor_prefs() {
		setMXJEditorFGColor(getMXJEditorFGColor().getRGB());
		setMXJEditorBGColor(getMXJEditorBGColor().getRGB());
		setMXJEditorComment1Color(getMXJEditorComment1Color().getRGB());
		setMXJEditorComment2Color(getMXJEditorComment2Color().getRGB());
		setMXJEditorKeyword1Color(getMXJEditorKeyword1Color().getRGB());
		setMXJEditorKeyword2Color(getMXJEditorKeyword2Color().getRGB());
		setMXJEditorKeyword3Color(getMXJEditorKeyword3Color().getRGB());
		setMXJEditorLiteral1Color(getMXJEditorLiteral1Color().getRGB());
		setMXJEditorLiteral2Color(getMXJEditorLiteral2Color().getRGB());
		setMXJEditorLabelColor(getMXJEditorLabelColor().getRGB());
		setMXJEditorOperatorColor(getMXJEditorOperatorColor().getRGB());
		setMXJEditorInvalidColor(getMXJEditorInvalidColor().getRGB());
		setMXJEditorBlockCaret(getMXJEditorBlockCaret());
		setMXJEditorCaretBlinks(getMXJEditorCaretBlinks());
		setMXJEditorEOLMarkerColor(getMXJEditorEOLMarkerColor().getRGB());
		setMXJEditorUseEOLMarkers(getMXJEditorUseEOLMarkers());
		setMXJEditorSelectionColor(getMXJEditorSelectionColor().getRGB());
		setMXJEditorLineHilite(getMXJEditorLineHilite());
		setMXJEditorLineHiliteColor(getMXJEditorLineHiliteColor().getRGB());
		setMXJEditorBracketHilite(getMXJEditorBracketHilite());
		setMXJEditorBracketHiliteColor(getMXJEditorBracketHiliteColor()
				.getRGB());
	}

}
