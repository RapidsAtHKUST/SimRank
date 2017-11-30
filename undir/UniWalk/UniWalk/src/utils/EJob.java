package utils;
import java.io.*;
import java.util.jar.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class EJob {
	public static String getLibPathValue(){
		StringBuffer res = new StringBuffer();
		try {
			File f = new File(".classpath");
			DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
			DocumentBuilder builder = factory.newDocumentBuilder();
			Document doc = builder.parse(f);
			NodeList nl = doc.getElementsByTagName("classpathentry");
			for (int i = 0; i < nl.getLength(); i++) {
				Node tmp = nl.item(i);
				String kind = tmp.getAttributes().getNamedItem("kind").getNodeValue();
				if(kind.equals("lib")){
					res.append( "./" + tmp.getAttributes().getNamedItem("path").getNodeValue() + " ");
				}
			}
		}catch(Exception e){
			e.printStackTrace();
		}
		System.out.println("package lib jar found in .classpath: " + res);
		return res.toString();
	}
	public static File createTempJar(String root) throws IOException {
        if (!new File(root).exists()) {
            return null;
        }
        
        Manifest manifest = new Manifest();
        manifest.getMainAttributes().putValue("Manifest-Version", "1.0");
        manifest.getMainAttributes().putValue("Class-Path", getLibPathValue());
        final File jarFile = File.createTempFile("EJob-", ".jar", new File(System
                .getProperty("java.io.tmpdir")));

        Runtime.getRuntime().addShutdownHook(new Thread() {
            public void run() {
                jarFile.delete();
            }
        });

        JarOutputStream out = new JarOutputStream(new FileOutputStream(jarFile),
                manifest);
        createTempJarInner(out, new File(root), "");
        createTempJarInner(out, new File("lib"), "lib");
        out.flush();
        out.close();
        return jarFile;
    }

    private static void createTempJarInner(JarOutputStream out, File f,
            String base) throws IOException {
        if (f.isDirectory()) {
            File[] fl = f.listFiles();
            if (base.length() > 0) {
                base = base + "/";
            }
            for (int i = 0; i < fl.length; i++) {
                createTempJarInner(out, fl[i], base + fl[i].getName());
            }
        } else {
            out.putNextEntry(new JarEntry(base));
            FileInputStream in = new FileInputStream(f);
            byte[] buffer = new byte[1024];
            int n = in.read(buffer);
            while (n != -1) {
                out.write(buffer, 0, n);
                n = in.read(buffer);
            }
            in.close();
        }
    }
    public static void main(String args[]) throws IOException{
    	System.out.println(System
                .getProperty("java.io.tmpdir"));
    	createTempJar("bin");
    }
}
