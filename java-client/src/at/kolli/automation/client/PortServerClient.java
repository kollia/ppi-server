/**
 *   This file is part of ppi-server.
 *
 *   ppi-server is free software: you can redistribute it and/or modify
 *   it under the terms of the Lesser GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   ppi-server is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   Lesser GNU General Public License for more details.
 *
 *   You should have received a copy of the Lesser GNU General Public License
 *   along with ppi-server.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * written by Alexander Kolli
 * 
 */
package at.kolli.automation.client;

import java.io.Console;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.LinkedList;
import java.util.Properties;

import org.eclipse.jface.dialogs.DialogSettings;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.eclipse.swt.SWT;
import org.eclipse.swt.SWTException;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

import org.apache.regexp.RE;
//import com.sun.org.apache.regexp.internal.RE;

import at.kolli.dialogs.DialogThread;
import at.kolli.layout.HtmTags;
import at.kolli.layout.WidgetChecker;

/**
 * Class to start application
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 02.12.2007
 * @since JDK 1.6
 */
public class PortServerClient 
{
	/**
	 * Instances of this class are responsible for managing the connection between SWT and the underlying operating system.
	 */
	private Display display;
	/**
	 * Instances of this class represent the "windows" which the desktop or "window manager" is managing
	 */
	private Shell toplevelShell;
	/**
	 * image buffer for system logo
	 */
	private Image[] m_aImgBuffer;
	
	/**
	 * main method of public PortServerClient to start application
	 * 
	 * @param args arguments from starting shell
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	public static void main(String[] args) 
	{
		boolean error= false;
		boolean shellpwd= false;
		LinkedList<String> param= new LinkedList<String>();
		String host;
		int port= 20004;
		String lang;
		
		HtmTags.debug= false;
		TreeNodes.m_sLayoutStyle= "desktop";
		for(String arg : args)
		{
			if(arg.substring(0, 1).equals("-"))
			{
				for(int c= 1; c < arg.length(); ++c)
				{
					String s= arg.substring(c, c+1);
					
					if(s.equals("?"))
					{
						System.out.println();
						System.out.println("syntax java -jar ppi-client [options]");
						System.out.println();
						System.out.println("       options:");
						System.out.println("           -?             -  show this help");
						System.out.println("           -f             -  full screen for main window");
						System.out.println("           -m             -  show no normaly menu in menubar (refresh/change user/exit)");
						System.out.println("           -t             -  displays no tree for sides, but in the menu-bar");
						System.out.println("           -w             -  main window without title-bar");
						System.out.println("           -d             -  show debug information and content of layout files");
						System.out.println("           -u <user>      -  starting binding to server with the given user name");
						System.out.println("           -p             -  password will be asking after command on shell");
						System.out.println("           -s <style>     -  fetch all layout files from server with this extension");
						System.out.println("                             default is desktop");
						System.out.println();
						System.out.println("       example:");
						System.out.println("          java -jar ppi-client -fmtwups <user> touchscreen");
						System.out.println("                     displays the main window as full screen with no tree and title");
						System.out.println("                     Fetch all layout files with extension touchscreen and start the client");
						System.out.println("                     with an user and the password will be asked on command line.");
						System.out.println("                     and also in the server client folder the touchscreen layout files are given.");
						System.out.println();
						System.out.println("            also the same command would be -fmtw -u <user> -p -s touchscreen");
						System.exit(0);
						
					}else if(s.equals("d"))
						HtmTags.debug= true;
					else if(s.equals("f"))
						HtmTags.fullscreen= true;
					else if(s.equals("m"))
						HtmTags.nomenu= true;
					else if(s.equals("t"))
						HtmTags.notree= true;
					else if(s.equals("w"))
						HtmTags.notitle= true;
					else if(s.equals("u"))
						param.addLast("u");
					else if(s.equals("p"))
						shellpwd= true;
					else if(s.equals("s"))
						param.addLast("s");
					else 
					{
						System.out.println("undefined param char '" + s + "`");
						error= true;
					}
				}
			}else if(param.size() != 0)
			{
				String s= param.getFirst();
				
				param.removeFirst();
				if(s.equals("u"))
					TreeNodes.m_sUser= arg;
				else if(s.equals("s"))
					TreeNodes.m_sLayoutStyle= arg;
			}else 
			{
				System.out.println("undefined parameter '" + arg + "`");
				error= true;
			}
		}
		if(error)
			System.exit(1);
		if(HtmTags.debug)
			System.out.println("start ppi-client");
		if(shellpwd)
		{
			Console c= System.console();
			
			if(c != null)
			{
				TreeNodes.m_sPwd= new String(c.readPassword("password: "));
			}else
			{
				int input= 0;
				
				try{
					while(input != 10)
					{
						TreeNodes.m_sPwd+= (char)input;
						input= System.in.read();
					}
				}catch(IOException ex)
				{
					System.out.println("ERROR on input password");
					return;
				}
			}
		}
		try{
			boolean bStop= false;
			String sPort;
			FileInputStream file= new FileInputStream("client.ini");
			Properties prop= new Properties();
			
			prop.load(file);
			lang= prop.getProperty("defaultLang");
			host= prop.getProperty("host");
			sPort= prop.getProperty("port");
			file.close();
			
			if(host == null)
			{
				System.out.println("ERROR: no host defined in client.ini");
				bStop= true;
			}
			if(sPort == null)
			{
				System.out.println("ERROR: no port defined in client.ini");
				bStop= true;
			}else
				port= Integer.parseInt(sPort);
			
			if(lang == null)
			{
				System.out.println("WARNING: no language defined in client.ini");
				if(!bStop)
					System.out.println("         use default englisch language");
				lang= "en";
			}
			if(bStop)
				System.exit(1);
			
		}catch(Exception ex)
		{
			System.out.println(ex);
			return;
		}
		String os;
		String homeEnv;
		String allUser;
		//RE linux= new RE("Linux");
		RE windows= new RE("Windows");
		File allUserPath, homePath;
		boolean bMsg= HtmTags.debug;
		boolean bError= false;
		
		os= System.getProperty("os.name");
		if(!windows.match(os))
		{
			homeEnv= System.getenv("HOME");
			if(homeEnv == null)
				homeEnv= "";
			allUser= homeEnv;
		}else // maybe it is Windows
		{
			homeEnv= System.getenv("HOMEDRIVE");
			if(homeEnv != null)
				homeEnv+= System.getenv("HOMEPATH");
			else
				homeEnv= "";
			allUser= homeEnv; //System.getenv("ALLUSERSPROFILE");
		}
		allUserPath= new File(allUser + File.separator + ".ppi-client" + File.separator + "layout");
		homePath= new File(homeEnv + File.separator + ".ppi-client");
		if(allUser.equals(""))
		{
			bError= true;
			bMsg= true;
		}
		if(bMsg)
		{
			System.out.println("found operating system '" + os + "'");
			System.out.println("with follow set path's:");
			System.out.println("    ALLUSER: 	'" + allUser + "'");
			System.out.println("    HOME:       '" + homeEnv + "'");
		}
		if(bError)
		{
			System.out.println("WARNING: can not found all user path");
			TreeNodes.m_bSaveLoacal= false;
		}else
		{
			if(!allUserPath.isDirectory())
			{
				if(!allUserPath.mkdirs())
				{
					System.out.println("WARNING: can not create localy path '~" + File.separator + ".ppi-client" + File.separator + "layout" + File.separator + "'");
					TreeNodes.m_bSaveLoacal= false;
				}
			}else
				TreeNodes.m_sLayoutPath= allUserPath.toString();
		}
		if(!homePath.isDirectory())
		{
			if(!homePath.mkdirs())
			{
				System.out.println("ERROR: can not create home path .ppi-client");
				System.exit(1);
			}
		}else
			TreeNodes.m_sHomePath= homePath.toString();
		
		TreeNodes.m_Settings= new DialogSettings("settings");
		if(!TreeNodes.loadSettings())
		{
			TreeNodes.m_Settings.put("version", 1.0);
		}
		
		new PortServerClient().startApp(host, port, lang);
		TreeNodes.saveSettings();
	}
	
	/**
	 * set images for top level shell
	 * 
	 * @param display handle of GUI-process
	 * @param topShell handle of top level shell
	 */
	private void setDisplayImages(Display display, Shell topShell)
	{
		String[] paths= {	"icons/memory16x16.png",
							"icons/memory32x32.png",
							"icons/memory48x48.png"	};
		Image img;
		LinkedList<Image> imgCollection= new LinkedList<Image>();

		for(int c= 0; c < paths.length; ++c)
		{
			try{
				img= new Image(display, paths[c]);
				imgCollection.add(img);
			}catch(SWTException ex)
			{
				// cannot define the image
				System.out.println("cannot found or load image " + paths[c]);
			}
		}
		if(imgCollection.size() > 0)
		{
			m_aImgBuffer= (Image[])imgCollection.toArray(new Image[0]);
			toplevelShell.setImages(m_aImgBuffer);
			for (Image sysImg : m_aImgBuffer) {
				
				sysImg.dispose();
			}
		}
	}
	
	/**
	 * starting hole application
	 * 
	 * @param host host-name or IP-address where the server running
	 * @param port port-number on which the server running on host
	 * @param lang language in which the messages and menu should displayed
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	protected void startApp(String host, int port, String lang)
	{
		int mainStyle= SWT.NONE;
		DialogThread dialog;
		WidgetChecker checker;
		MsgClientConnector client;
		LayoutLoader loader;
		MsgTranslator trans= MsgTranslator.init(lang);

		if(!trans.getError().equals("NONE"))
		{
			System.out.println(trans.getError());
			System.exit(1);
		}

		if(HtmTags.notitle)
			mainStyle|= SWT.NO_TRIM;
		else
			mainStyle|= SWT.TITLE | SWT.MIN | SWT.CLOSE | SWT.RESIZE;
		display= new Display();
		toplevelShell= new Shell(display, mainStyle);
		dialog= DialogThread.instance(toplevelShell);
		LayoutLoader.init(toplevelShell, host, port);

		if(HtmTags.fullscreen)
			toplevelShell.setMaximized(true);
		setDisplayImages(display, toplevelShell);
		dialog.needProgressBar();
		if(	TreeNodes.m_sUser.equals("")
			||
			TreeNodes.m_sPwd.equals("")	)
		{
			dialog.show(trans.translate("dialogConnectionTitle"), trans.translate("dialogUserVerification"));
			dialog.needUserVerificationFields();
		}
		loader= LayoutLoader.instance();
		loader.setState(LayoutLoader.CREATE);
		if(dialog.produceDialog(LayoutLoader.CREATE).equals(DialogThread.states.CANCEL))
		{
			loader.stopThread();
			return;
		}
		toplevelShell.addDisposeListener(new DisposeListener() {
		
			public void widgetDisposed(DisposeEvent ev) {

				int[] sashWeight;
				Point location, size;
				IDialogSettings login= TreeNodes.m_Settings.getSection(TreeNodes.m_sLayoutStyle);
				LayoutLoader loader= LayoutLoader.instance();

				location= toplevelShell.getLocation();
				size= toplevelShell.getSize();
				if(!HtmTags.notree)
				{
					sashWeight= loader.m_shellForm.getWeights();

					login.put("sashwidth", sashWeight[0]);
					login.put("sashheight", sashWeight[1]);
				}
				login.put("xLocation", location.x);
				login.put("yLocation", location.y);
				login.put("mainwidth", size.x);
				login.put("mainheight", size.y);
		
			}
		
		});
		toplevelShell.open();
		
		while(!toplevelShell.isDisposed())
		{
			if(!display.readAndDispatch())
				display.sleep();
		}

		
		client= MsgClientConnector.instance();
		client.closeConnection();
		checker= WidgetChecker.instance();
		try{
			//dialog.stopping();
			checker.stopping();
			checker.join();
			loader.stopThread();
		}catch(InterruptedException ex)
		{
			System.out.println("Interrupted exception by ending with join");
			ex.printStackTrace();
		}
		display.dispose();
		/*if(m_aImgBuffer != null)
		{
			for (Image sysImg : m_aImgBuffer) {
				
				sysImg.dispose();
			}
		}*/
	}
}
