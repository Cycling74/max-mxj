package com.cycling74.max;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.net.*;
import java.util.Vector;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.jar.*;
import java.util.*;
import java.security.*;

public class MXJClassLoaderImpl extends ClassLoader {
	private Vector fPathItems;
	private final HashMap code_resource_map = new HashMap(64);
	private String _extended_search_dir = null;
	private String[] _pruned_search_path = null;
	private static final String SEP = String.valueOf(File.separatorChar);

	public MXJClassLoaderImpl() {
		fPathItems = new Vector(16);
	}

	public MXJClassLoaderImpl(Vector classpath) {
		fPathItems = new Vector(16);
		for (int i = 0; i < classpath.size(); i++)
			addDirectory((String) classpath.elementAt(i));

	}

	public MXJClassLoaderImpl(Vector classpath, String extended_search_dir) {
		fPathItems = new Vector(16);
		for (int i = 0; i < classpath.size(); i++)
			addDirectory((String) classpath.elementAt(i));
		_extended_search_dir = extended_search_dir;
	}

	protected void setExtendedClassSearchDirectory(String dirname) {
		_extended_search_dir = dirname;
	}

	public String stripSlash(String str) {
		if (str.length() > 0 && str.charAt(str.length() - 1) == '/') {
			str = str.substring(0, str.length() - 1);
		}
		return str;
	}

	protected void addDirectory(String dirname) {
		boolean isjar = false;
		if (dirname.endsWith(".jar") || dirname.endsWith(".zip")) {
			isjar = true;
		} else {
			// make sure it ends with a slash for formalities sake
			if (dirname.charAt(dirname.length() - 1) != File.separatorChar)
				dirname = dirname + String.valueOf(File.separatorChar);
		}

		File f = new File(dirname);
		if (f.exists()) {
			if (isjar) {
				try { // we will give zips a shot....
					definepackages(new JarFile(dirname)); // we need to make
					// sure packages are
					// defined b4
					// defining a class
					// with
				} catch (IOException e) {
					System.err
					.println("(mxj classloader) problems loading dynamic classpath entry."
							+ dirname);
					e.printStackTrace();
					return;// don't add it mostlikely an invalid jar
				}
			} // this classloader
			// add it if it exists
			fPathItems.addElement(dirname);
		} else {
			System.out.println("(mxj classloader) dynamic classpath entry "
					+ dirname + " does not exist.");
		}
	}

	public Class loadClass(String name, boolean resolve)
			throws ClassNotFoundException {
		return doLoadClass(name, resolve, true);
	}

	public synchronized Class doLoadClass(String name, boolean resolve,
			boolean report_error) throws ClassNotFoundException {
		Class c = findLoadedClass(name);
		// Class c = null;
		if (c != null)
			return c;
		//
		// Delegate the loading of excluded classes to the
		// standard class loader.
		//

		try {
			c = findSystemClass(name);
			return c;
		} catch (ClassNotFoundException e) {
			// keep searching
		}

		if (c == null) {
			Object[] data = lookupClassData(name);
			if (data == null) {
				if (report_error)
					System.err.println("(mxj) classloader unable to find "
							+ name);
			} else {
				c = defineClass(name, (byte[]) data[0], 0,
						((byte[]) data[0]).length, (ProtectionDomain) data[1]);
				if (resolve)
					resolveClass(c);
			}
		}

		return c;
	}

	private Object[] lookupClassData(String className)
			throws ClassNotFoundException {
		byte[] data = null;
		for (int i = 0; i < fPathItems.size(); i++) {
			String path = (String) fPathItems.elementAt(i);
			String fileName = null;
			CodeSource cs = null;
			if (isJar(path)) {
				fileName = className.replace('.', '/') + ".class";
				data = loadJarData(path, fileName);
			} else {
				fileName = className.replace('.', File.separatorChar)
						+ ".class";
				data = loadFileData(path, fileName);

			}

			if (data != null) {
				try {
					cs = new CodeSource(new URL("file://" + path),
							(java.security.cert.Certificate[]) null);
				} catch (MalformedURLException mfe) {
					mfe.printStackTrace();
				}
				ProtectionDomain pd = new ProtectionDomain(cs, null, this, null);

				return new Object[] { data, pd };
			}
		}

		Object[] lastchance = null;
		String repl = (className.replace('.', File.separatorChar) + ".class");

		// first look in the contextual search path (project or collective, as
		// relevant)
		String[] pds = MaxSystem.getSearchPathForContext();
		if (pds != null) {
			for (int i = 0; i < pds.length; i++) {
				String pd = pds[i];
				if (pd != null) {
					File f = new File(pd);
					if (!f.exists() || !f.isDirectory()) {
						System.err
						.println("(mxj classloader) contextual search path directory "
								+ pd
								+ " does not seem to exist or is not directory!");
					} else {
						lastchance = look_for_class_recursive(pd, repl);
						if (lastchance != null)
							return lastchance;
					}
				}
			}
		}
		// then look in the default path
		String pd = MaxSystem.getDefaultPath();
		if (pd != null) {
			File f = new File(pd);
			if (!f.exists() || !f.isDirectory()) {
				System.err.println("(mxj classloader) default directory " + pd
						+ " does not seem to exist or is not directory!");
			} else {
				String maxpath = stripSlash(MaxSystem.maxPathToNativePath("./"));
				if (maxpath.equalsIgnoreCase(pd)) {
					// non-recursive search if we're in the Max application
					// folder
					lastchance = look_for_class(pd, repl);
				} else {
					// recursive search if we're not
					lastchance = look_for_class_recursive(pd, repl);
				}
			}
		}

		if (lastchance != null)
			return lastchance;

		// lastchance is null look in search path
		// ..first prune and cache search path dirs
		if (_pruned_search_path == null) {
			set_pruned_search_path();
		}
		// actually look in search path
		for (int i = 0; i < _pruned_search_path.length; i++) {
			// System.err.println("class. _pruned_search_path: " +
			// _pruned_search_path[i]);
			lastchance = look_for_class_recursive(_pruned_search_path[i], repl);
			if (lastchance != null)
				return lastchance;
		}

		// can't seem to find you little classi class....
		throw new ClassNotFoundException(className);
	}

	private void set_pruned_search_path() {
		String[] sp = MaxSystem.getSearchPath();
		String[] candidates = new String[128];
		String maxpath = stripSlash(MaxSystem.maxPathToNativePath("./"));
		int cc = 0;// candidate count
		boolean skip;
		for (int i = 0; i < sp.length; i++) {
			String pot = sp[i];
			skip = false;
			if (maxpath.equalsIgnoreCase(pot)) {
				continue; // exclude the application dir
			}
			for (int ii = 0; ii < cc; ii++) {
				// check to see if we already have a parent of this directory
				// in our candidates list i.e. compare /Applications/Max MSP4.5/
				// /Applications/Max MSP 4.5/examples/
				// and don't add the child
				if (pot.startsWith(candidates[ii])) {
					skip = true;
					break;
				}
			}
			if (!skip)
				candidates[cc++] = pot;
		}
		String[] muster = new String[cc];
		System.arraycopy(candidates, 0, muster, 0, cc);
		_pruned_search_path = muster;
	}

	private Object[] look_for_class(String dir, String classfile) {
		return look_for_class_impl(dir, classfile, false);
	}

	private Object[] look_for_class_recursive(String dir, String classfile) {
		return look_for_class_impl(dir, classfile, true);
	}

	private Object[] look_for_class_impl(String dir, String classfile,
			boolean recursive) {
		byte[] data = null;
		CodeSource cs = null;

		if (dir == null)
			return null;

		if (!dir.endsWith(SEP))
			dir = dir + SEP;

		String currentfile = dir + classfile;
		if ((new File(currentfile)).exists()) {
			data = loadFileData(dir, classfile);
			if (data != null) {
				try {
					cs = new CodeSource(new URL("file://" + dir),
							(java.security.cert.Certificate[]) null);
				} catch (MalformedURLException mfe) {
					mfe.printStackTrace();
				}
				ProtectionDomain pd = new ProtectionDomain(cs, null, this, null);
				return new Object[] { data, pd };
			} else {
				System.err
				.println("(mxj classloader) unable to load class file bytes from "
						+ currentfile);
				return null;
			}
		}

		if (!recursive)
			return null;

		String[] fl = (new File(dir)).list();
		if (fl == null)
			return null; // File.list return null on IO error!!
		for (int i = 0; i < fl.length; i++) {
			String f = new String(dir + fl[i]);

			if (f.endsWith(".jar") || f.endsWith(".zip")) {

				String jarname = classfile.replace(File.separatorChar, '/');// jars
				// always
				// use
				// /
				// path
				// style
				data = loadJarData(f, jarname);
				if (data != null) {
					try {
						cs = new CodeSource(new URL("file://" + f),
								(java.security.cert.Certificate[]) null);
					} catch (MalformedURLException mfe) {
						mfe.printStackTrace();
					}
					ProtectionDomain pd = new ProtectionDomain(cs, null, this,
							null);
					return new Object[] { data, pd };
				} else
					continue;
			} else if ((new File(f)).isDirectory()) {
				Object[] ret = look_for_class_recursive(f, classfile);
				if (ret != null)
					return ret;
			}
		}
		return null;
	}

	/*
	 * This is where we attempt to look for classes as files or in jars in the
	 * currently loading patcher directory --toph
	 */
	private Object[] look_for_class_extended(String className) {
		byte[] data = null;
		File f;
		String fileName = null;
		CodeSource cs = null;

		if (_extended_search_dir == null
				|| (!(f = new File(_extended_search_dir)).exists() && !f
						.isDirectory()))
			return null;

		fileName = className.replace('.', File.separatorChar) + ".class";
		data = loadFileData(_extended_search_dir, fileName);
		if (data != null) {

			try {
				cs = new CodeSource(new URL("file://" + _extended_search_dir),
						(java.security.cert.Certificate[]) null);
			} catch (MalformedURLException mfe) {
				mfe.printStackTrace();
			}
			ProtectionDomain pd = new ProtectionDomain(cs, null, this, null);
			return new Object[] { data, pd };

		} else {
			// look for jars
			fileName = className.replace('.', '/') + ".class";
			String[] files = f.list();
			for (int i = 0; i < files.length; i++) {
				if (!isJar(files[i]))
					continue;

				data = loadJarData(_extended_search_dir + File.separatorChar
						+ files[i], fileName);
				if (data != null) {
					try {
						cs = new CodeSource(new URL("file://"
								+ _extended_search_dir + "/" + files[i]),
								(java.security.cert.Certificate[]) null);
					} catch (MalformedURLException mfe) {
						mfe.printStackTrace();
					}
					ProtectionDomain pd = new ProtectionDomain(cs, null, this,
							null);
					return new Object[] { data, pd };
				}
			}
		}
		// didn't find it here either
		return null;
	}

	boolean isJar(String pathEntry) {
		return pathEntry.endsWith(".jar") || pathEntry.endsWith(".zip");
	}

	private byte[] loadFileData(String path, String fileName) {
		File file = new File(path, fileName);
		if (file.exists()) {
			return getClassData(file);
		} else {
			return null;
		}
	}

	// Returns the contents of the file in a byte array.
	private byte[] getClassData(File file) {
		InputStream is = null;
		try {
			is = new FileInputStream(file);
			// Get the size of the file
			long length = file.length();
			// You cannot create an array using a long type.
			// It needs to be an int type.
			// Before converting to an int type, check
			// to ensure that file is not larger than Integer.MAX_VALUE.
			if (length > Integer.MAX_VALUE) {
				System.err.println(file.getAbsolutePath()
						+ " is too big to be read");
				is.close();
				return null;
			}

			// Create the byte array to hold the data
			byte[] bytes = new byte[(int) length];
			// Read in the bytes
			int offset = 0;
			int numRead = 0;
			while (offset < bytes.length
					&& (numRead = is.read(bytes, offset, bytes.length - offset)) >= 0) {
				offset += numRead;
			}
			// Ensure all the bytes have been read in
			if (offset < bytes.length)
				throw new IOException("Could not completely read file "
						+ file.getName());

			// Close the input stream and return bytes
			is.close();
			return bytes;
		} catch (FileNotFoundException fne) {
			return null;
		} catch (IOException e) {
			if (is != null)
				try {
					is.close();
				} catch (IOException e1) {
					e1.printStackTrace();
				}
			e.printStackTrace();
			return null;
		}

	}

	private byte[] loadJarData(String path, String fileName) {
		ZipFile zipFile = null;
		InputStream stream = null;
		File archive = new File(path);

		if (!archive.exists())
			return null;
		try {
			zipFile = new ZipFile(archive);
		} catch (IOException io) {
			return null;
		}
		ZipEntry entry = zipFile.getEntry(fileName);

		/*
		 * DEBUG SHIT - TML java.util.Enumeration e = null; e =
		 * zipFile.entries(); while(e.hasMoreElements()) { entry =
		 * (ZipEntry)e.nextElement(); System.out.println("entry is "+entry);
		 * 
		 * }
		 */

		if (entry == null)
			return null;
		int size = (int) entry.getSize();
		try {
			stream = zipFile.getInputStream(entry);
			byte[] data = new byte[size];
			int pos = 0;
			while (pos < size) {
				int n = stream.read(data, pos, data.length - pos);
				pos += n;
			}

			zipFile.close();
			return data;
		} catch (IOException ee) {

			try {
				if (zipFile != null) {
					zipFile.close();// this will close any streams opened using
					// the
					// zip file as well
				}
			} catch (IOException eee) {
			}
		}
		return null;
	}

	public String[] getCurrentClassPath() {
		boolean ext_search;
		int size, i;
		String[] ret;

		ext_search = (_extended_search_dir != null) ? true : false;
		if (ext_search) {
			size = fPathItems.size() + 1;
			ret = new String[size];
			for (i = 0; i < size - 1; i++)
				ret[i] = (String) fPathItems.elementAt(i);
			ret[i] = _extended_search_dir;
		} else {
			size = fPathItems.size();
			ret = new String[size];
			for (i = 0; i < size; i++)
				ret[i] = (String) fPathItems.elementAt(i);
		}

		return ret;
	}

	public void dump() {
		String[] s = getCurrentClassPath();
		for (int i = 0; i < s.length; i++)
			System.out.println("  " + s[i]);
	}

	/* define all the packages in a given jar */
	private void definepackages(JarFile f) {
		try {
			Manifest manifest = f.getManifest();
			if (manifest != null) {
				Map entries = manifest.getEntries();
				Iterator i = entries.keySet().iterator();
				while (i.hasNext()) {
					String path = (String) i.next();
					if (!path.endsWith(".class")) {
						String name = path.replace('/', '.');
						if (name.endsWith("."))
							name = name.substring(0, name.length() - 1);
						// code url not implemented
						definePackage(path, name, manifest, null);
					}
				}
			}
		} catch (Exception e) {
		}
	}

	private Package definePackage(String path, String name, Manifest man,
			URL url) throws IllegalArgumentException {
		String specTitle = null;
		String specVersion = null;
		String specVendor = null;
		String implTitle = null;
		String implVersion = null;
		String implVendor = null;
		String sealed = null;
		URL sealBase = null;

		Attributes attr = man.getAttributes(path);
		if (attr != null) {
			specTitle = attr.getValue(Attributes.Name.SPECIFICATION_TITLE);
			specVersion = attr.getValue(Attributes.Name.SPECIFICATION_VERSION);
			specVendor = attr.getValue(Attributes.Name.SPECIFICATION_VENDOR);
			implTitle = attr.getValue(Attributes.Name.IMPLEMENTATION_TITLE);
			implVersion = attr.getValue(Attributes.Name.IMPLEMENTATION_TITLE);
			implVersion = attr.getValue(Attributes.Name.IMPLEMENTATION_VERSION);
			implVendor = attr.getValue(Attributes.Name.IMPLEMENTATION_VENDOR);
			sealed = attr.getValue(Attributes.Name.SEALED);
		}
		attr = man.getMainAttributes();

		if (attr != null) {
			if (specTitle == null)
				specTitle = attr.getValue(Attributes.Name.SPECIFICATION_TITLE);
			if (specVersion == null)
				specVersion = attr
				.getValue(Attributes.Name.SPECIFICATION_VERSION);
			if (specVendor == null)
				specVendor = attr
				.getValue(Attributes.Name.SPECIFICATION_VENDOR);
			if (implTitle == null)
				implTitle = attr.getValue(Attributes.Name.IMPLEMENTATION_TITLE);
			if (implVersion == null)
				implVersion = attr
				.getValue(Attributes.Name.IMPLEMENTATION_VERSION);
			if (implVendor == null)
				implVendor = attr
				.getValue(Attributes.Name.IMPLEMENTATION_VENDOR);
			if (sealed == null)
				sealed = attr.getValue(Attributes.Name.SEALED);
		}
		return super.definePackage(name, specTitle, specVersion, specVendor,
				implTitle, implVersion, implVendor, sealBase);
	}

	public URL getResource(String name) {
		return lookup_resource(name);
	}

	private URL lookup_resource(String rname) {

		URL ret = null;
		// look for resource in system classpath
		ret = ClassLoader.getSystemResource(rname);
		if (ret != null)
			return ret;

		// look for resource in dynamic classpath
		File f = null;
		ZipFile zipFile = null;
		ZipEntry entry = null;
		for (int i = 0; i < fPathItems.size(); i++) {
			String path = (String) fPathItems.elementAt(i);
			if (isJar(path)) {
				try {
					zipFile = new ZipFile(path);
					entry = zipFile.getEntry(rname);
				} catch (IOException ioe) {
					ioe.printStackTrace();
					continue;
				}
				if (entry == null)
					continue;
				else {
					try {
						ret = new URL("jar:file:" + path + "!/" + rname);
					} catch (MalformedURLException mfue) {
						mfue.printStackTrace();
						return null;
					}
					return ret;
				}
			} else {
				if (!path.endsWith(SEP))
					path = path + SEP;
				if ((f = new File(path + rname)).exists()) {
					try {
						ret = f.toURL();
					} catch (MalformedURLException mfue) {
						mfue.printStackTrace();
						return null;
					}
					return ret;
				}

			}
		}

		// first look in the contextual search path (project or collective, as
		// relevant)
		String[] pds = MaxSystem.getSearchPathForContext();
		if (pds != null) {
			for (int i = 0; i < pds.length; i++) {
				String pd = pds[i];
				if (pd != null) {
					f = new File(pd);
					if (!f.exists() || !f.isDirectory()) {
						System.err
						.println("(mxj classloader) contextual search path directory "
								+ pd
								+ " does not seem to exist or is not directory!");
					} else {
						ret = look_for_resource_recursive(pd, rname);
						if (ret != null)
							return ret;
					}
				}
			}
		}

		// look in default path
		String pd = MaxSystem.getDefaultPath();
		if (pd != null) {
			f = new File(pd);
			if (!f.exists() || !f.isDirectory()) {
				System.err
				.println("(mxj classloader look for resource) default directory "
						+ pd
						+ " does not seem to exist or is not directory!");
			} else {
				String maxpath = stripSlash(MaxSystem.maxPathToNativePath("./"));
				if (maxpath.equalsIgnoreCase(pd)) {
					// non-recursive search if we're in the Max application
					// folder
					ret = look_for_resource(pd, rname);
				} else {
					// recursive search if we're not
					ret = look_for_resource_recursive(pd, rname);
				}
			}
		}

		if (ret != null)
			return ret;

		// ret is null look in search path
		// ..first prune and cache search path dirs
		if (_pruned_search_path == null) {
			set_pruned_search_path();
		}
		// actually look in search path
		for (int i = 0; i < _pruned_search_path.length; i++) {
			// System.err.println("resource. _pruned_search_path[i]: " +
			// _pruned_search_path[i]);
			ret = look_for_resource_recursive(_pruned_search_path[i], rname);
			if (ret != null)
				return ret;
		}

		// can't seem to find you little resource....
		return null;

	}

	private URL look_for_resource(String dir, String rname) {
		return look_for_resource_impl(dir, rname, false);
	}

	private URL look_for_resource_recursive(String dir, String rname) {
		return look_for_resource_impl(dir, rname, true);
	}

	private URL look_for_resource_impl(String dir, String rname,
			boolean recursive) {
		URL ret = null;
		File file = null;
		if (dir == null)
			return null;

		if (!dir.endsWith(SEP))
			dir = dir + SEP;

		String currentfile = dir + rname;
		if (((file = new File(currentfile))).exists()) {
			try {
				ret = file.toURL();
			} catch (MalformedURLException mfue) {
				mfue.printStackTrace();
				return null;
			}
			return ret;
		}

		if (!recursive)
			return null;

		String[] fl = (new File(dir)).list();
		if (fl == null)
			return null; // File.list return null on IO error!!
		ZipFile zipFile = null;
		ZipEntry entry = null;
		for (int i = 0; i < fl.length; i++) {
			String f = new String(dir + fl[i]);

			if (f.endsWith(".jar") || f.endsWith(".zip")) {
				try {
					zipFile = new ZipFile(f);
					entry = zipFile.getEntry(rname);
				} catch (IOException ioe) {
					ioe.printStackTrace();
					continue;
				}
				if (entry == null)
					continue;
				else {
					try {
						ret = new URL("jar:file:" + f + "!/" + rname);
					} catch (MalformedURLException mfue) {
						mfue.printStackTrace();
						return null;
					}
					return ret;
				}
			} else if ((new File(f)).isDirectory()) {
				ret = look_for_resource_recursive(f, rname);
				if (ret != null)
					return ret;
			}
		}
		return null;
	}

	// We may need to make these more sophisicated like in JEdit JarClassLoader
	public InputStream getResourceAsStream(String name) {
		for (int i = 0; i < fPathItems.size(); i++) {
			String path = (String) fPathItems.elementAt(i);
			ZipFile zipFile = null;
			if (isJar(path)) {
				try {
					zipFile = new ZipFile(path);
					ZipEntry entry = zipFile.getEntry(name);
					if (entry == null)
						continue;
					else
						return zipFile.getInputStream(entry);

				} catch (IOException io) {
					System.err.println("(mxj classloader) getResourceAsStream");
					io.printStackTrace();
					continue;
				}

			} else {
				File file = new File(path, name);
				if (file.exists()) {
					try {
						return new FileInputStream(file);
					} catch (Exception ee) {
						System.err
						.println("(mxj classloader) getResourceAsStream");
						ee.printStackTrace();
						return null;
					}
				}

			}
		}
		// let the system classloader have a go at it
		return ClassLoader.getSystemResourceAsStream(name);
	}

}
