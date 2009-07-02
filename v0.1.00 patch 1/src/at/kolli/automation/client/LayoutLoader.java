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
 * created from Alexander Kolli
 * for porject: ppi-server-java-client
 * package: at.kolli.automation.client
 * file: LayoutLoader.java
 * date: 23.12.2007 
 */
package at.kolli.automation.client;

import java.awt.Toolkit;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Set;
import java.util.StringTokenizer;

import org.eclipse.jface.dialogs.DialogSettings;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseMoveListener;
import org.eclipse.swt.events.MouseTrackListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.TreeAdapter;
import org.eclipse.swt.events.TreeEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeItem;

import com.sun.org.apache.regexp.internal.RE;

import at.kolli.dialogs.DialogThread;
import at.kolli.dialogs.DisplayAdapter;
import at.kolli.dialogs.DialogThread.states;
import at.kolli.layout.Component;
import at.kolli.layout.HtmTags;
import at.kolli.layout.PopupMenu;
import at.kolli.layout.WidgetChecker;

/**
 * generate layout sides
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 23.12.2007
 * @since JDK 1.6
 */
public class LayoutLoader extends Thread
{
	/**
	 * constant variable or waiting for next action
	 */
	public static final short WAIT= 0;
	/**
	 * constant variable of creating first layout of widgets
	 */
	public static final short CREATE= 1;
	/**
	 * constant variable of new user verification and refresh layout
	 */
	public static final short UPDATE= 2;
	/**
	 * constant variable to refresh content
	 */
	public static final short REFRESH= 3;
	/**
	 * type of layout creation<br />
	 * CREATE for first beginning<br />
	 * UPDATE for refresh content with allocate new user
	 * REFRESH only for refresh content
	 */
	private Short type= 0;
	/**
	 * whether tread should stopping
	 */
	private Boolean m_bStop= false;
	/**
	 * instance of LayoutLoader
	 */
	private static LayoutLoader _instance= null;
	/**
	 * stack layout of main widget on the right side
	 */
	private StackLayout m_StackLayout= new StackLayout();
	public static Shell m_oTopLevelShell= null;
	/**
	 * first composite inside the top level shell
	 */
	private static Composite mainComposite;
	public static String m_sHost;
	public static int m_nPort;
	//private String m_sLang;
	/**
	 * tree on the left side in the top level shell
	 */
	private static Tree m_oTree;
	/**
	 * relative offset position of user-array inside the shell 
	 */
	public static volatile Point mainRelPoint;
	/**
	 * composite for popup menu if notree be set
	 */
	private static Composite m_oPopupComposite= null;
	/**
	 * composite for all sides witch should switches
	 */
	private static Composite m_oMainComposite;
	public SashForm m_shellForm= null;
	/**
	 * array of all root nodes
	 */
	private ArrayList<TreeNodes> m_aTreeNodes= new ArrayList<TreeNodes>();
	/**
	 * actual visible node on the screen
	 */
	private TreeNodes m_oAktTreeNode= null;
	/**
	 * name of actual node on the screen
	 */
	public String m_sAktFolder= "";
	/**
	 * all components from the visible node
	 */
	private ArrayList<Component> m_aoComponents= null;
	/**
	 * width of main-window if saved on hard disk
	 */
	private int m_nWidth;
	/**
	 * height of main-window if saved on hard disk
	 */
	private int m_nHeight;
	/**
	 * private instantiation of LayoutLoader
	 * 
	 * @param topLevelShell Shell of main-window
	 * @param host host on which server running
	 * @param port on which port server does running
	 * @author Alexander Kolli
	 * @version 1.00.00, 23.12.2007
	 * @since JDK 1.6
	 */
	private LayoutLoader(Shell topLevelShell, String host, int port)
	{
		super();
		m_oTopLevelShell= topLevelShell;
		m_sHost= host;
		m_nPort= port;
		//m_sLang= lang;
	}
	
	/**
	 * create and initialize LayoutLoader if not exist before
	 * 
	 * @param topLevelShell Shell of main-window
	 * @param host host on which server running
	 * @param port on which port server does running
	 * @return true if LayoutLoader be created, otherwise if it exist before false
	 * @author Alexander Kolli
	 * @version 1.00.00, 23.12.2007
	 * @since JDK 1.6
	 */
	public static boolean init(Shell topLevelShell, String host, int port)
	{
		if(_instance == null)
		{
			_instance= new LayoutLoader(topLevelShell, host, port);
			_instance.start();
			return true;
		}
		return false;
	}
	
	/**
	 * returning instance of LayoutLoader
	 * 
	 * @return instance of LayoutLoader
	 * @author Alexander Kolli
	 * @version 1.00.00, 23.12.2007
	 * @since JDK 1.6
	 */
	public static LayoutLoader instance()
	{
		return _instance;
	}
	
	/**
	 * stopping thread
	 */
	public void stopThread()
	{
		synchronized (m_bStop) {
			
			m_bStop= true;
		}
	}
	
	/**
	 * whether thread should stopping
	 * 
	 * @return true if thread should stopping
	 */
	public boolean stopping()
	{
		boolean stop;
		
		synchronized (m_bStop) {
			
			stop= m_bStop;
		}
		return stop;
	}
	
	/**
	 * getter method for type
	 * 
	 * @return actual state of LayoutLoader
	 */
	public short getType()
	{
		short type;

		synchronized (this.type) {
			
			type= this.type;
		}
		return type;
	}
	/**
	 * run routine to load files from server
	 * and generate layout files
	 * 
	 * @serial
	 * @see
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 23.12.2007
	 * @since JDK 1.6
	 */
	public void run()
	{
		Boolean bConnected= false;
		short type= 0;
		String user= "";
		String pwd= "";
		String olduser, oldpwd;
		String error= "";
		HashMap<String, Date> folder= new HashMap<String, Date>();
		DialogThread dialog= DialogThread.instance();
		MsgTranslator trans= MsgTranslator.instance();
		NoStopClientConnector client;
		String loginname;
		IDialogSettings login= TreeNodes.m_Settings.getSection("loginDialog");
		
		setName("LayoutLoader");
		while(!stopping())
		{
			while(getType() == 0)
			{
				try{
					sleep(100);
				}catch(InterruptedException ex)
				{
					System.out.println("cannot sleep 10 milliseconds for wating on LayoutLoader-therad");
					ex.printStackTrace();
				}
				if(stopping())
					return;
			}
			while(	!dialog.isOpen()
					&&
					getType() != 0	)
			{
				try{
					sleep(100);
				}catch(InterruptedException ex)
				{
					
				}
				if(stopping())
					return;
			}
			type= getType();
			if(type == 0)
				continue;
			bConnected= false;
			client= NoStopClientConnector.instance(m_sHost, m_nPort);
			olduser= user= TreeNodes.m_sUser;
			oldpwd= pwd= TreeNodes.m_sPwd;
			if(	type != REFRESH
				&&
				login != null	)
			{
				loginname= null;
				if(type == LayoutLoader.UPDATE)
					loginname= login.get("lastchangename");
				if(loginname == null)
					loginname= login.get("lastloginname");
				user= loginname;
			}
			while(!bConnected)
			{
				if(	type == CREATE
					||
					type == UPDATE	)
				{
					if(dialog.verifyUser(user, pwd, error).equals(states.CANCEL))
						break;
					user= TreeNodes.m_sUser;
				}		
				dialog.needProgressBar();
				dialog.show(trans.translate("dialogConnectionTitle"), trans.translate("dialogConnectionSearchMsg"));
				
				if(type != REFRESH)
				{
					if(type == CREATE)
						client.openConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd);
					else
						client.changeUser(TreeNodes.m_sUser, TreeNodes.m_sPwd);
					if(client.hasError())
					{
						Toolkit oKit= Toolkit.getDefaultToolkit();
						
						oKit.beep();
						error= client.getErrorCode();
						dialog.show(client.getErrorMessage());
						
					}else
						bConnected= true;
				}else
					bConnected= true;
			}
	
			if(dialog.dialogState().equals(states.RUN))
			{
				user= TreeNodes.m_sUser;
				if(login == null)
				{
					login= new DialogSettings("loginDialog");
					login.put("lastloginname", user);
					login.put("loginnames", user);
					TreeNodes.m_Settings.addSection(login);
				}else
				{
					Boolean exists= false;
					String names= login.get("loginnames");
					String[] snames= names.split(":");
					
					for (String name : snames)
					{
						if(name.equals(user))
						{
							exists= true;
							break;
						}
					}
					if(!exists)
						login.put("loginnames", names + ":" + user);
					if(type == CREATE)
						login.put("lastloginname", user);
					else if(type == UPDATE)
						login.put("lastchangename", user);
				}
				
				folder= client.getDirectory("." + TreeNodes.m_sLayoutStyle);
				if(!client.hasError())
				{
					if(HtmTags.debug)
					{
						Set<String> folderKeys= folder.keySet();
						
						for(String str : folderKeys)
							System.out.println(str);
					}
				}else
				{
					System.out.println(client.getErrorMessage());
					return;
				}
		
				final short itype= type;
				TreeNodes.m_hmDirectory= folder;
				/*Display.getDefault().syncExec(new Runnable()
				{
				
					//@Override
					public void run()
					{
						if(itype == CREATE)
							initializeMainWidget(TreeNodes.m_hmDirectory);
						else
							updateMainWidget(TreeNodes.m_hmDirectory);
					}
				
				});*/
				if(itype == CREATE)
					initializeMainWidget(TreeNodes.m_hmDirectory);
				else
					updateMainWidget(TreeNodes.m_hmDirectory);
			
				if(dialog.dialogState().equals(states.RUN))
				{
					//System.out.println("Close Dialog Box");
					dialog.close();
				}else
					// change user back;
					client.changeUser(olduser, oldpwd);
			}
			synchronized (this.type) {
				
				this.type= 0;
				type= 0;
			}
		}
	}

	public boolean setActSideVisible()
	{
		TreeNodes oldNode= m_oAktTreeNode;
		WidgetChecker checker= WidgetChecker.instance();
		
		if(m_oAktTreeNode != null)
			m_oAktTreeNode.setInvisible();
		for(TreeNodes node : m_aTreeNodes)
		{
			m_oAktTreeNode= node.setVisible(m_StackLayout, m_sAktFolder);
			
			if(m_oAktTreeNode != null)
			{
				if(	m_aoComponents != null
					&&
					m_aoComponents.size() > 0	)
				{
					if(HtmTags.debug)
						System.out.println("remove listeners from side " + m_sAktFolder);
					DisplayAdapter.syncExec(new Runnable() {
						
						public void run() {
							
							for(final Component component : m_aoComponents)
							{
								component.removeListeners();
							}
						}
						
					}, "LayoutLoader::setActiveSideVisible() removeListeners()");	
				}
				m_aoComponents= m_oAktTreeNode.getComponents();
				if(	m_aoComponents != null
					&&
					m_aoComponents.size() > 0	)
				{
					if(HtmTags.debug)
						System.out.println("add listeners by side " + m_sAktFolder);
					DisplayAdapter.syncExec(new Runnable() {
						
						public void run() {
							
							for(final Component component : m_aoComponents)
							{										
								component.addListeners();
							}
						}						
					}, "LayoutLoader::setActiveSideVisible() addListeners()");						
				}
				checker.setTreeNode(m_oAktTreeNode);
				return true;
			}
		}
		m_oAktTreeNode= oldNode;
		return false;
	}
	
	/**
	 * returning the absolute position of the first composite inside the shell
	 * 
	 * @return point of position
	 */
	public static Point getAbsoluteUseFieldPoint()
	{
		Rectangle 	userField,
					shellField;
		Point pos;
		
		shellField= m_oTopLevelShell.getBounds();
		userField= mainComposite.getBounds();
		pos= new Point(shellField.x + userField.x, shellField.y + userField.y + (shellField.height - userField.height));
		return pos;
	}
	
	protected void execInitialize()
	{
		String check;
		Menu menuBar;
		MsgTranslator trans= MsgTranslator.instance();
		FillLayout mainLayout= new FillLayout();
		FillLayout treeLayout= new FillLayout();

		IDialogSettings login;
		//final Composite mainComposite;
		final Group treeComposite;				
		int sashWeight[]= { 200, 800 };
		Rectangle monitor;
		int xLocation;
		int yLocation;
		
		// create menu
		if(	HtmTags.nomenu )
			//&&
			//!HtmTags.notree	)
		{
			menuBar= null;
		}else
		{
			menuBar= new Menu(m_oTopLevelShell, SWT.BAR);
			m_oTopLevelShell.setMenuBar(menuBar);
		}
		if(!HtmTags.nomenu)
		{
			MenuItem menuTitle= new MenuItem(menuBar, SWT.CASCADE);
			Menu menuList1= new Menu(m_oTopLevelShell, SWT.DROP_DOWN);
			MenuItem refreshItem= new MenuItem(menuList1, SWT.NULL);
			MenuItem changeUserItem= new MenuItem(menuList1, SWT.NULL);
			new MenuItem(menuList1, SWT.SEPARATOR);
			MenuItem exitItem= new MenuItem(menuList1, SWT.NULL);
			
			menuTitle.setText(trans.translate("menu_top_app"));
			// first menu list
			menuTitle.setMenu(menuList1);			
			refreshItem.setText(trans.translate("menu_app_refresh"));
			changeUserItem.setText(trans.translate("menu_app_changeUser"));
			exitItem.setText(trans.translate("menu_app_exit"));
			
			// event-handling for MenuItem's
			refreshItem.addSelectionListener(new SelectionAdapter() {
			
				@Override
				public void widgetSelected(SelectionEvent e) {

					LayoutLoader loader= LayoutLoader.instance();
					DialogThread dialog= DialogThread.instance(m_oTopLevelShell);
					MsgTranslator trans= MsgTranslator.instance();
					
					super.widgetSelected(e);
					synchronized (TreeNodes.m_DISPLAYLOCK)
					{
						loader.start(LayoutLoader.REFRESH);
						dialog.needProgressBar();
						dialog.needUserVerificationFields();
						dialog.show(trans.translate("dialogChangeUser"), trans.translate("dialogUserVerification"));
						dialog.produceDialog(REFRESH);
						loader.start(LayoutLoader.WAIT);
					}
				}
			
			});
			changeUserItem.addSelectionListener(new SelectionAdapter() {
			
				@Override
				public void widgetSelected(SelectionEvent e) {
					
					LayoutLoader loader= LayoutLoader.instance();
					DialogThread dialog= DialogThread.instance(m_oTopLevelShell);
					MsgTranslator trans= MsgTranslator.instance();
					
					super.widgetSelected(e);
					synchronized (TreeNodes.m_DISPLAYLOCK)
					{
						loader.start(LayoutLoader.UPDATE);
						dialog.needProgressBar();
						dialog.needUserVerificationFields();
						dialog.show(trans.translate("dialogChangeUser"), trans.translate("dialogUserVerification"));
						dialog.produceDialog(UPDATE);
					}
					if(HtmTags.debug)
						System.out.println("change user to '" + TreeNodes.m_sUser + "'");
				}
			
			});
			exitItem.addSelectionListener(new SelectionAdapter() {
			
				@Override
				public void widgetSelected(SelectionEvent e) {
					
					super.widgetSelected(e);
					m_oTopLevelShell.close();
				}
			
			});
		}
		
		// create composites to display
		login= TreeNodes.m_Settings.getSection(TreeNodes.m_sLayoutStyle);
		if(login == null)
		{
			login= new DialogSettings(TreeNodes.m_sLayoutStyle);
			TreeNodes.m_Settings.addSection(login);
		}
		mainComposite= new Composite(m_oTopLevelShell, SWT.NONE);
		if(HtmTags.notree)
		{
			GridLayout layout= new GridLayout();
			RowLayout popupLayout= new RowLayout();
			
			m_oPopupComposite= new Composite(mainComposite, SWT.NONE);
			m_oMainComposite= new Composite(mainComposite, SWT.NONE);
			treeComposite= null;
			m_shellForm= null;
			m_oTree= null;
			mainComposite.setLayout(layout);
			m_oPopupComposite.setLayout(popupLayout);
			
		}else
		{
			m_shellForm = new SashForm(mainComposite, SWT.HORIZONTAL);
			treeComposite= new Group(m_shellForm, SWT.SHADOW_ETCHED_IN);
			m_oMainComposite= new Composite(m_shellForm, SWT.NONE);
			m_oTree= new Tree(treeComposite, SWT.SINGLE);

			mainLayout.marginHeight= 10;
			mainLayout.marginWidth= 10;
			mainComposite.setLayout(mainLayout);

			check= login.get("sashwidth");
			if(check != null)
			{
				sashWeight[0]= login.getInt("sashwidth");
				sashWeight[1]= login.getInt("sashheight");
			}
			m_shellForm.setLayout(new FillLayout());
			m_shellForm.setWeights(sashWeight);
		}
		
		if(!HtmTags.notree)
		{
			treeLayout.marginWidth= 1;
			treeLayout.marginHeight= 1;
			treeComposite.setLayout(treeLayout);		
			treeComposite.setVisible(true);
		}
		
		m_StackLayout.marginHeight= 10;
		m_StackLayout.marginWidth= 10;
		m_oMainComposite.setLayout(m_StackLayout);

		m_oTopLevelShell.setLayout(new FillLayout());
		m_oTopLevelShell.setText("ppi-client  (physical port interface client)");

		monitor= Display.getDefault().getPrimaryMonitor().getBounds();
		if(HtmTags.fullscreen)
		{
			m_nWidth= monitor.width;
			m_nHeight= monitor.height;
		}else
		{	
			check= login.get("xLocation");
			if(check != null)
			{
				xLocation= login.getInt("xLocation");
				yLocation= login.getInt("yLocation");
				m_nWidth= login.getInt("mainwidth");
				m_nHeight= login.getInt("mainheight");
			}else
			{
				m_nWidth= 0;
				m_nHeight= 0;
				xLocation= monitor.width / 2;
				yLocation= monitor.height / 2;
				monitor= m_oTopLevelShell.getBounds();
				xLocation-= ( monitor.width / 2 );
				yLocation-= ( monitor.height / 2 );
			}
			m_oTopLevelShell.setLocation(xLocation, yLocation);
		}
		if(!HtmTags.notree)
		{// add listeners for tree
			
			m_oTree.addSelectionListener(new SelectionAdapter()
			{
				public void widgetSelected(SelectionEvent e)
				{
					synchronized (TreeNodes.m_DISPLAYLOCK)
					{
						TreeItem items[]= m_oTree.getSelection();
						TreeItem akt= null;
						String name= "";
		
						if(items.length > 0)
						{
							akt= items[0];
							//m_sAktFolder= akt.getText();
						}//else
						//	m_sAktFolder= "";
						while(akt != null)
						{
							name= ":" + akt.getText() + name;
							akt= akt.getParentItem();
						}
						if(name.length() > 1)
							name= name.substring(1);
						if(HtmTags.debug)
							System.out.println("Treenode "+ name + " is selected");
						m_sAktFolder= name;
						setActSideVisible();
					}
				}
			});
			
			if(HtmTags.debug)
			{
				m_oTree.addTreeListener(new TreeAdapter()
				{
					public void treeCollapsed(TreeEvent e)
					{
						if(HtmTags.debug)
							System.out.println("collapsed node: "+((TreeItem) e.item).getText());
					}
					public void treeExpanded(TreeEvent e)
					{
						if(HtmTags.debug)
							System.out.println("expanded node: "+((TreeItem) e.item).getText());
					}
				});
			}
		}
	}
	/**
	 * method starting and manage the begin of the widget on display
	 * 
	 * @param folder all exist folder which should displayed in the nodes
	 */
	protected void initializeMainWidget(HashMap<String, Date> folderMap)
	{
		NoStopClientConnector client= NoStopClientConnector.instance();
		final WidgetChecker checker;
		final Set<String> folderSet= folderMap.keySet();
		
		DisplayAdapter.syncExec(new Runnable() {
		
			public void run() 
			{				
				execInitialize();
			}
		
		});
		m_aTreeNodes= creatingWidgets(m_oTree, m_oMainComposite, folderSet);

		if(	m_nWidth != 0
			&&
			m_nHeight != 0	)
		{
			DisplayAdapter.syncExec(new Runnable() {
			
				public void run() 
				{
					m_oTopLevelShell.setSize(m_nWidth, m_nHeight);
				}
			});
		}
		client.secondConnection();
		checker= WidgetChecker.instance(m_aTreeNodes.get(0));
		checker.start();
		
		if(HtmTags.notree)
		{
			synchronized (TreeNodes.m_DISPLAYLOCK)
			{
				setFirstSide(m_aTreeNodes, "");
			}
		}
	}
	/**
	 * set first side by starting active
	 * 
	 * @param list of tree nodes
	 * @param path for tree node
	 * @return whether an side was set active
	 */
	private boolean setFirstSide(ArrayList<TreeNodes> nodes, String path)
	{
		for(TreeNodes current : nodes)
		{
			m_sAktFolder= path + current.getName();
			if(setActSideVisible())
				return true;
		}
		for(TreeNodes current : nodes)
		{
			if(setFirstSide(current.getChilds(), path + current.getName() + ":"))
				return true;
		}
		return false;
	}
	/**
	 * initial LayoutLoader for working
	 * 
	 * @param type CREATE, UPDATE or REFRESH layout
	 */
	public void start(short type)
	{
		synchronized (this.type) {
		
			this.type= type;
			//this.type.notify();
		}
	}
	
	/**
	 * method update and manage the sides content of the widget to display
	 * 
	 * @param folder all exist folder which should displayed in the nodes
	 */
	private void updateMainWidget(HashMap<String, Date> folderMap)
	{
		Set<String> folderSet= folderMap.keySet();
		ArrayList<TreeNodes> nodes= creatingWidgets(m_oTree, m_oMainComposite, folderSet);	
		DialogThread dialog= DialogThread.instance(m_oTopLevelShell);

		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
		{
			for (TreeNodes node : nodes) {
				
				node.dispose();
			}
			return;
		}
		DisplayAdapter.syncExec(new Runnable() {
			
			public void run() {
				
				for(final Component component : m_aoComponents)
				{
					component.removeListeners();
				}
			}
			
		});	
		for (TreeNodes node : m_aTreeNodes) {
			
			node.dispose();
		}
		m_aTreeNodes= nodes;
		if(!setActSideVisible())
		{
			m_sAktFolder= nodes.get(0).getName();
			setActSideVisible();
		}
		//System.out.println("End or update --------------");
	}
	/**
	 * initialize content of widgets
	 * 
	 * @param start whether initialization is for first creation
	 * @param tree side tree on the left side
	 * @param subComposite mani composite for inserting all content of sides
	 * @param folderSet all side names
	 * @return ArrayList of created TreeNodes
	 */
	private ArrayList<TreeNodes> creatingWidgets(final Tree tree, Composite subComposite, Set<String> folderSet)
	{
		boolean access;
		short pos= 0;
		TreeNodes node= null;
		DialogThread dialog= DialogThread.instance(m_oTopLevelShell);
		String folderName, aktName= "";
		ArrayList<String> folder= new ArrayList<String>();
		ArrayList<String> aktFolderList= new ArrayList<String>();
		ArrayList<TreeNodes> oRetTrees= new ArrayList<TreeNodes>();
		RE nameParser= new RE("^(.*)\\." + TreeNodes.m_sLayoutStyle);
		int nMax= dialog.getMaximum();
		
		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			return oRetTrees;
		if(folderSet.size() > 0)
			DialogThread.m_nProgressSteps= nMax / folderSet.size();
		else
			DialogThread.m_nProgressSteps= nMax;
		dialog.setSelection(0);
		for(String setStr : folderSet)
		{
			if(nameParser.match(setStr))
				folder.add(nameParser.getParen(1));
			else
				folder.add(setStr);
		}
		Collections.sort(folder);
		for(String folderStr : folder)
		{
			StringTokenizer token= new StringTokenizer(folderStr, "/");
			
			folderName= token.nextToken();
			if(nameParser.match(folderName))
				folderName= nameParser.getParen(1);
			if(!aktName.isEmpty())
			{
				if(!folderName.equals(aktName))
				{	
					access= true;
					try{
						if(HtmTags.notree)
							node= new TreeNodes(pos, m_oPopupComposite, subComposite, tree, aktFolderList);
						else
							node= new TreeNodes(pos, subComposite, tree, aktFolderList);
						
					}catch(IllegalAccessException ex)
					{
						if(ex.getMessage().equals("no side access"))
						{
							access= false;
						}
					}
					if(access)
					{
						oRetTrees.add(node);
						++pos;
					}
					if(dialog.dialogState().equals(DialogThread.states.CANCEL))
						return oRetTrees;
					aktFolderList.clear();
					aktName= folderName;
				}
				aktFolderList.add(folderStr);
			}else
			{
				aktName= folderName;
				aktFolderList.add(folderStr);
			}
		}
		access= true;
		try{
			if(HtmTags.notree)
				node= new TreeNodes(pos, m_oPopupComposite, subComposite, tree, aktFolderList);
			else
				node= new TreeNodes(pos, subComposite, tree, aktFolderList);
			
		}catch(IllegalAccessException ex)
		{
			if(ex.getMessage().equals("no side access"))
			{
				access= false;
			}
		}
		if(access)
			oRetTrees.add(node);
		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			return oRetTrees;
		dialog.setSelection(nMax);
		
		return oRetTrees;			
	}
}
