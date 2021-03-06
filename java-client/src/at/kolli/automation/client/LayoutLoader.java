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
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Set;
import java.util.SortedSet;
import java.util.StringTokenizer;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.eclipse.jface.dialogs.DialogSettings;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ControlListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.TreeAdapter;
import org.eclipse.swt.events.TreeEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeItem;

import org.apache.regexp.RE;

import at.kolli.dialogs.DialogThread;
import at.kolli.dialogs.DisplayAdapter;
import at.kolli.dialogs.DialogThread.states;
import at.kolli.layout.HtmTags;
import at.kolli.layout.IComponentListener;
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
	 * constant variable of waiting for next action
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
	 * constant variable when connection was broken
	 */
	public static final short BROKEN= 4;
	/**
	 * constant variable for ending client
	 */
	public static final short DISPOSE= 5;
	/**
	 * type of layout creation<br />
	 * CREATE for first beginning<br />
	 * UPDATE for refresh content with allocate new user<br />
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
	 * lock object to create conditions
	 */
	private final Lock stateLock= new ReentrantLock();
	/**
	 * condition to wait if more threads want to open the dialog box
	 */
	private final Condition newStateCondition= stateLock.newCondition();
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
	 * composite which inherit pop-up navigation bar
	 */
	private static Composite m_oPopupIn= null;
	/**
	 * composite for popup menu if notree be set
	 */
	private static Composite m_oPopupComposite= null;
	/**
	 * composite for all sides witch should switches
	 */
	private static Composite m_oMainComposite;
	/**
	 * sash form where split tree and viewer
	 */
	public SashForm m_shellForm= null;
	/**
	 * whether main window is initialized
	 */
	private boolean m_bInitialized= false;
	/**
	 * lock object for sides
	 */
	public static final Lock sideLock= new ReentrantLock();
	/**
	 * array of all root nodes
	 */
	private ArrayList<TreeNodes> m_aTreeNodes= null;
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
	private ArrayList<IComponentListener> m_aoComponents= null;
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
			stateLock.lock();
			newStateCondition.signalAll();
			stateLock.unlock();
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

		stateLock.lock();
		type= this.type;
		stateLock.unlock();
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
		MsgClientConnector client;
		String loginname;
		IDialogSettings login= TreeNodes.m_Settings.getSection("loginDialog");
		
		setName("LayoutLoader");
		client= MsgClientConnector.instance(m_sHost, m_nPort);
		while(!stopping())
		{
			try{
				while(getType() == WAIT)
				{
					stateLock.lock();	
					try{
						// wait until no new content should be loading
							newStateCondition.await();
					}catch(InterruptedException ex)
					{	
						System.out.println("newStateCodition get's InterruptedException, so wait for 100 milliseconds");
						try{
							sleep(100);
						}catch(InterruptedException iex)
						{
							System.out.println("cannot sleep 10 milliseconds for wating on LayoutLoader-thread");
							iex.printStackTrace();
						}
					}
					finally
					{
						stateLock.unlock();
					}
					if(stopping())
						return;
				}
				while(	!dialog.isOpen()
						&&
						getType() != WAIT	)
				{
					try{
						sleep(100);
					}catch(InterruptedException ex)
					{
						
					}
					if(stopping())
						return;
				}
				MsgTranslator.instance().clearErrorPool();
				type= getType();
				if(type == DISPOSE)
				{
					dialog.setSelection(0);
					dialog.setSteps(1);
					if(HtmTags.showDisposeClient)
					{
						if(	m_aTreeNodes != null &&
							!m_aTreeNodes.isEmpty()	)
						{
							TreeNodes.m_nNodeCount= 0;
							for(TreeNodes node : m_aTreeNodes)
								TreeNodes.m_nNodeCount+= node.count(); 
							dialog.setSteps(TreeNodes.m_nNodeCount + 1);
							for(TreeNodes node : m_aTreeNodes)
								node.dispose(/*subnodes*/true);
						}
					}
					dialog.show(trans.translate("closeConnections"));
					//client.closeConnection();
					dialog.nextStep();
					dialog.close();
					return;
				}
				if(type == WAIT)
					continue;
				bConnected= false;
				olduser= user= TreeNodes.m_sUser;
				oldpwd= pwd= TreeNodes.m_sPwd;
				if(	type != REFRESH &&
					type != BROKEN &&
					login != null		)
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
					if(	type == CREATE ||
						type == UPDATE		)
					{
						if(dialog.verifyUser(user, pwd, error).equals(states.CANCEL))
							break;
						user= TreeNodes.m_sUser;
					}		
					dialog.needProgressBar();
					dialog.show(trans.translate("dialogConnectionTitle"), trans.translate("dialogConnectionSearchMsg"));
					
					if(type != REFRESH)
					{
						if(	type == CREATE ||
							type == BROKEN		)
						{
							client.openNewConnection(TreeNodes.m_sUser, TreeNodes.m_sPwd);
						}else
							client.changeUser(TreeNodes.m_sUser, TreeNodes.m_sPwd, /*bthrow*/true);
						if(!dialog.isOpen())
							stopThread();
						if(stopping())
							break;
						type= getType();
						if(client.hasError())
						{
							Toolkit oKit= Toolkit.getDefaultToolkit();
							
							oKit.beep();
							error= client.getErrorCode();
							dialog.show(client.getErrorMessage());
							client.closeConnection();
							
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
					
					folder= client.getDirectory("." + TreeNodes.m_sLayoutStyle, /*bthrow*/true);
					if(!client.hasError())
					{
						if(HtmTags.debug)
						{
							ArrayList<String> folderKeys= new ArrayList<String>(folder.keySet());
							
							Collections.sort(folderKeys);
							System.out.println("found follow Files on server:");
							for(String str : folderKeys)
								System.out.println(str);
						}
					}else
					{
						System.out.println(client.getErrorMessage());
						return;
					}
			
					TreeNodes.m_hmDirectory= folder;
					dialog.show(trans.translate("dialogLoadContent"));
					initializeMainWidget(TreeNodes.m_hmDirectory);
					if(dialog.dialogState().equals(DialogThread.states.CANCEL))
						return;
					if(dialog.dialogState().equals(states.RUN))
					{
						//System.out.println("Close Dialog Box");
						dialog.close();
					}else
						// change user back;
						client.changeUser(olduser, oldpwd, /*bthrow*/true);
				}
				/**
				 * create all not currently visible sides
				 * in background with lower or higher priority
				 */
				if(HtmTags.useBackgroundLoadingPriority > 0)
				{
					Thread.currentThread().setPriority(HtmTags.useBackgroundLoadingPriority);
					if(createSidesOnBackground())
					{
						setState(WAIT);
						type= WAIT;
					}
					Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
					if(HtmTags.debug)
						System.out.println("reading of sides on background was finished");
				}
				setState(WAIT);
				
			}catch(IOException ex)
			{
				if(HtmTags.debug)
				{
					System.out.println("-----------------------------------------------");
					System.out.println(client.getErrorMessage());
					ex.printStackTrace();
					System.out.println("-----------------------------------------------");
				}
				client.closeConnection();
				setState(BROKEN);
				type= WAIT;
			}	
		}
		if(HtmTags.debug)
			System.out.println("Ending of thread '" + getName() + "'");
	}
	
	/**
	 * create all other sides on background
	 * 
	 * @return whether all sides was created
	 */
	private boolean createSidesOnBackground() throws IOException
	{
		
		for (TreeNodes node : m_aTreeNodes)
		{
			//if(HtmTags.debug)
				System.out.println("create side '" + node.getName() + "' on background");
			if(!node.createPage())
				return false;
			if(!createSubSidesOnBackground(node))
				return false;
		}
		return true;
	}

	/**
	 * create all other sub-sides from TreeNode on background
	 * 
	 * @return whether all sides was created
	 */
	private boolean createSubSidesOnBackground(TreeNodes mainNode) throws IOException
	{
		ArrayList<TreeNodes> oNodes;
		
		oNodes= mainNode.getChilds();
		for (TreeNodes node : oNodes)
		{
			//if(HtmTags.debug)
				System.out.println("create sub side '" + node.getName() + "' on background");
			if(!node.createPage())
				return false;
			if(!createSubSidesOnBackground(node))
				return false;
		}
		return true;
	}

	/**
	 * check whether new value should activate an new side
	 * 
	 * @param cont container with new value on which result of folder and subroutine
	 * @return new side name when should be activated
	 */
	public String checkNewSide(NodeContainer cont)
	{
		String sRv, sub;
		
		sRv= "";
		if(	cont != null &&
			cont.hasDoubleValue()	)
		{
			if(cont.getDValue() > 0)
			{
				sub= cont.getFolderName() + ":" + cont.getSubroutineName();
				for(TreeNodes node : m_aTreeNodes)
				{
					sRv= node.getPageFrom(sub);
					if(!sRv.equals(""))
						break;
				}
			}
		}
		return sRv;
	}
	
	/**
	 * set defined side in <code>m_sAktFolder</code> to new active side
	 * 
	 * @param parent parent side
	 * @param whether should inform server want to set page active also when node has no body
	 * @return whether can set new page or was also before active
	 */
	public boolean setCurrentSideVisible(boolean inform)
	{
		return setCurrentNodeSideVisible(m_aTreeNodes, "", inform);
	}
	/**
	 * set defined side in <code>m_sAktFolder</code> to new active side
	 * 
	 * @param parent parent side
	 * @param whether should inform server want to set page active also when node has no body
	 * @return whether can set new page or was also before active
	 */
	public boolean setCurrentNodeSideVisible(ArrayList<TreeNodes> nodes, String parent, boolean inform)
	{
		TreeNodes newNode;
		TreeNodes oldNode;
		ArrayList<TreeNodes> subNodes;
		WidgetChecker checker= WidgetChecker.instance();
		Thread t= null;
		
		if(HtmTags.lockDebug)
		{
			t= Thread.currentThread();
			System.out.println(t.getName()+" want to lock sideLock for setActiveSideVisible");
		}
		sideLock.lock();
		if(	HtmTags.lockDebug)
		{
			System.out.println(t.getName()+" lock sideLock for setActiveSideVisible");
		}
		oldNode= m_oAktTreeNode;
		if(	m_oAktTreeNode != null &&
			m_oAktTreeNode.isCurrentSidePath(m_sAktFolder)	)
		{
			if(HtmTags.lockDebug	)
			{
				System.out.println(t.getName()+" unlock sideLock for setActiveSideVisible");
			}
			sideLock.unlock();
			return true;
		}
		newNode= null;
		for(TreeNodes node : nodes)
		{
			if(	m_sAktFolder.equals(parent + node.getName()) ||
				m_oAktTreeNode != null ||		// ask only whether not should shown inside tree
				node.treeDisplay()			) 	// by first starting of client when no actual tree node defined
			{
				newNode= node.setVisible(m_StackLayout, m_sAktFolder, inform);
			}
			
			if(newNode != null)
			{
				if(	m_aoComponents != null &&
					m_aoComponents.size() > 0	)
				{
					for(TreeNodes page : nodes)
						page.hearOnSides(/*firstDef*/false);
					if(m_oAktTreeNode != null)
						m_oAktTreeNode.setInvisible();
					m_oAktTreeNode= newNode;
					if(HtmTags.debug)
						System.out.println("remove listeners from side " + node.getName());
					DisplayAdapter.syncExec(new Runnable() {
						
						public void run() {
							
							for(final IComponentListener component : m_aoComponents)
							{
								component.removeListeners();
							}
						}
						
					}, "LayoutLoader::setActiveSideVisible() removeListeners()");	
					
				}else
				{
					if(m_oAktTreeNode != null)
						m_oAktTreeNode.setInvisible();
					m_oAktTreeNode= newNode;
					for(TreeNodes page : nodes)
						page.hearOnSides(/*firstDef*/true);
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
							
							for(final IComponentListener component : m_aoComponents)
							{
								try{
									component.addListeners();
								}catch(IOException ex)
								{
							    	if(	HtmTags.debug)
							    	{
							    		MsgTranslator trans= MsgTranslator.instance();
							    		
							    		System.out.println("ERROR: connection to server is broken by trying to set node");
							    		System.out.println("       " + trans.translate(ex.getMessage()));
							    	}
								}
						    }							
						}						
					}, "LayoutLoader::setActiveSideVisible() addListeners()");					
				}
				checker.setTreeNode(m_oAktTreeNode);
				if(	HtmTags.lockDebug	)
				{
					System.out.println(t.getName()+" unlock sideLock for setActiveSideVisible");
				}
				sideLock.unlock();
				return true;
			}
			subNodes= node.getChilds();
			if(	subNodes != null &&
				!subNodes.isEmpty() &&
				setCurrentNodeSideVisible(subNodes, parent + node.getName() + "/", inform)	)
			{
				return true;
			}
		}
		m_oAktTreeNode= oldNode;
		if(	HtmTags.lockDebug	)
		{
			System.out.println(t.getName()+" unlock sideLock for setActiveSideVisible");
		}
		sideLock.unlock();
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
						loader.setState(LayoutLoader.REFRESH);
						dialog.needProgressBar();
						dialog.needUserVerificationFields();
						dialog.show(trans.translate("dialogChangeUser"), trans.translate("dialogUserVerification"));
						dialog.produceDialog(REFRESH);
						loader.setState(LayoutLoader.WAIT);
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
						loader.setState(LayoutLoader.UPDATE);
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
					
					LayoutLoader loader= LayoutLoader.instance();
					
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
		
		mainComposite= new Composite(m_oTopLevelShell, SWT.NONE);
		mainComposite.setBackground(HtmTags.systemColor);
		if(HtmTags.notree)
		{
			FillLayout fill= new FillLayout();
			RowLayout popupLayout= new RowLayout();
			
			m_shellForm = new SashForm(mainComposite, SWT.VERTICAL);
			m_oPopupIn= new Composite(m_shellForm, SWT.NONE);
			m_oPopupComposite= new Composite(m_oPopupIn, SWT.NONE);
			m_oMainComposite= new Composite(m_shellForm, SWT.NONE);
			
			m_shellForm.setBackground(HtmTags.systemColor);
			m_oPopupIn.setBackground(HtmTags.systemColor);
			m_oPopupComposite.setBackground(HtmTags.systemColor);
			m_oMainComposite.setBackground(HtmTags.systemColor);
			fill.marginHeight= 0;
			fill.marginWidth= 0;
			popupLayout.marginHeight= HtmTags.popupPadding;
			popupLayout.marginWidth= HtmTags.popupPadding;
			popupLayout.marginBottom= 0;
			popupLayout.marginTop= 0;
			popupLayout.marginLeft= 0;
			popupLayout.marginRight= 0;
			mainComposite.setLayout(mainLayout);
			m_oPopupIn.setLayout(fill);
			m_oPopupComposite.setLayout(popupLayout);
			treeComposite= null;
			m_oTree= null;
			
		}else
		{
			m_shellForm = new SashForm(mainComposite, SWT.HORIZONTAL);
			treeComposite= new Group(m_shellForm, SWT.SHADOW_ETCHED_IN);
			m_oMainComposite= new Composite(m_shellForm, SWT.NONE);
			m_oTree= new Tree(treeComposite, SWT.SINGLE);

			treeComposite.setBackground(HtmTags.systemColor);
			m_oMainComposite.setBackground(HtmTags.systemColor);
			mainLayout.marginHeight= 0;
			mainLayout.marginWidth= 0;
			mainComposite.setLayout(mainLayout);

			check= login.get("sashwidth");
			if(check != null)
			{
				sashWeight[0]= login.getInt("sashwidth");
				sashWeight[1]= login.getInt("sashheight");
			}
			m_shellForm.setWeights(sashWeight);
			m_shellForm.setLayout(new FillLayout());
		}
		
		if(!HtmTags.notree)
		{
			treeLayout.marginWidth= 1;
			treeLayout.marginHeight= 1;
			treeComposite.setLayout(treeLayout);		
			treeComposite.setVisible(true);
		}
		
		m_StackLayout.marginHeight= HtmTags.mainPadding;
		m_StackLayout.marginWidth= HtmTags.mainPadding;
		m_oMainComposite.setLayout(m_StackLayout);

		m_oTopLevelShell.setLayout(new FillLayout());
		m_oTopLevelShell.setText("ppi-client  (physical port interface client)");

		if(!HtmTags.notree)
		{// add listeners for tree
			
			m_oTree.addSelectionListener(new SelectionAdapter()
			{
				public void widgetSelected(SelectionEvent e)
				{
					synchronized (TreeNodes.m_DISPLAYLOCK)
					{
						ArrayList<TreeNodes> searchNodes;
						ArrayList<TreeItem> itemTree= new ArrayList<TreeItem>();
						TreeItem existItems[]= m_oTree.getItems();
						TreeItem items[]= m_oTree.getSelection();
						TreeItem curItem= null;
						int pos, count;
						String name= "";
		
						if(items.length > 0)
							curItem= items[0];
						else
							return;
						while(curItem != null)
						{
							itemTree.add(0, curItem);
							curItem= curItem.getParentItem();
							
						}
						searchNodes= m_aTreeNodes;						
						do{
							curItem= itemTree.get(0);
							itemTree.remove(0);
							pos= 0;
							for (TreeItem s : existItems)
							{
								if(s == curItem)
								{
									existItems= s.getItems();
									break;
								}
								++pos;
							}
							count= -1;
							for (TreeNodes node : searchNodes)
							{
								if(node.treeDisplay())
									++count;
								if(count == pos)
								{
									name+= "/" + node.getName();
									searchNodes= node.getChilds();
									break;
								}
							}
						}while(	itemTree.size() > 0 &&
								searchNodes != null		);
						if(name.length() > 1)
							name= name.substring(1);
						if(HtmTags.debug)
							System.out.println("Treenode "+ name + " is selected");
						m_sAktFolder= name;
						setCurrentSideVisible(/*inform server by no body*/true);
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
	 * dispose all components created by starting
	 */
	public void disposeClientEnding()
	{
		DialogThread dialog= DialogThread.instance(m_oTopLevelShell);
		MsgTranslator trans= MsgTranslator.instance();

			new Thread(new Runnable() {
				
				@Override
				public void run() {

					WidgetChecker checker;
					MsgClientConnector client;

					checker= WidgetChecker.instance();
					checker.stopping();
					client= MsgClientConnector.instance();
					client.closeConnection();
					try{
						if(checker.isAlive())
							checker.join();
					}catch(InterruptedException ex)
					{
						System.out.println("Interrupted exception by ending checker thread with join");
						ex.printStackTrace();
					}
				}
			}).start();
			//dialog.stopping();
			dialog.needProgressBar();
			dialog.show(trans.translate("dialogDispose"), "");
			setState(LayoutLoader.DISPOSE);
			dialog.produceDialog(DISPOSE);
			stopThread();
			
/*		setState(LayoutLoader.DISPOSE);
		dialog.needProgressBar();
		dialog.show(trans.translate("dialogDispose"), "");
		dialog.produceDialog(DISPOSE);
		setState(LayoutLoader.WAIT);*/
	}
	/**
	 * method starting and manage the begin of the widget on display
	 * 
	 * @param folder all exist folder which should displayed in the nodes
	 */
	protected void initializeMainWidget(HashMap<String, Date> folderMap) throws IOException
	{
		MsgClientConnector client= MsgClientConnector.instance();
		DialogThread dialog= DialogThread.instance(m_oTopLevelShell);
		ArrayList<TreeNodes> nodes;	
		final WidgetChecker checker;
		final Set<String> folderSet= folderMap.keySet();
		
		if(!m_bInitialized)
		{// when no tree nodes exists
		 // initialization of main window cannot be done
			if(dialog.dialogState().equals(DialogThread.states.CANCEL))
				return;
			DisplayAdapter.syncExec(new Runnable() {
			
				public void run() 
				{				
					execInitialize();
				}			
			});
			m_bInitialized= true;
			
		}else
		{
			// remove all Listeners from actually side
			if(	m_aoComponents == null &&
				m_oAktTreeNode != null		)
			{				
				m_aoComponents= m_oAktTreeNode.getComponents();
			}
			if(m_aoComponents != null)
			{
				if(dialog.dialogState().equals(DialogThread.states.CANCEL))
					return;
				DisplayAdapter.syncExec(new Runnable() {
					
					public void run() {
						
						for(final IComponentListener component : m_aoComponents)
						{
							component.removeListeners();
						}
						m_aoComponents= null;
					}
					
				});
			}
		}
		nodes= creatingWidgets(m_oTree, m_oMainComposite, folderSet);
		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			return;
		if(m_aTreeNodes != null)
		{
			for (TreeNodes node : m_aTreeNodes) 
			{
				boolean found= false;
				
				for (TreeNodes inode : nodes)
				{
					if(node == inode)
					{
						found= true;
						break;
					}
				}
				if(!found)
					node.dispose(/*subnodes*/true);
			}
		}
		if(HtmTags.debug)
			System.out.println("all new nodes created inside class LayoutLoader");
		m_aTreeNodes= nodes;
		if(HtmTags.debug)
			System.out.println("all new nodes changed with old nodes");

		if(	m_nWidth != 0
			&&
			m_nHeight != 0	)
		{
			if(dialog.dialogState().equals(DialogThread.states.CANCEL))
				return;
			DisplayAdapter.syncExec(new Runnable() {
			
				public void run() 
				{
					m_oTopLevelShell.setSize(m_nWidth, m_nHeight);
					if(m_oPopupComposite != null)
					{
						int sashHeight[]= { 200, 800 };
						Rectangle size, pop;
						Control[] controls;
						
						m_oPopupComposite.pack(true);
						controls= m_oPopupComposite.getChildren();
						if(	controls != null &&
							controls.length > 0	)
						{
							pop= m_oPopupComposite.getChildren()[0].getBounds();
						}else
						{
							pop= m_oPopupComposite.getBounds();
						}
						size= m_shellForm.getBounds();
						sashHeight[0]= pop.height + HtmTags.popupPadding * 2;
						sashHeight[1]= size.height - pop.height - HtmTags.mainPadding;
						m_shellForm.setWeights(sashHeight);
						//m_shellForm.pack();
					}
				}
			});
		}
		if(HtmTags.notree)
		{
			DisplayAdapter.syncExec(new Runnable() {
				
				public void run() 
				{
					m_oTopLevelShell.addControlListener(new ControlListener() {
						
						@Override
						public void controlResized(ControlEvent arg0) {

							int sashHeight[]= { 200, 800 };
							Rectangle size, pop;
							Control[] controls;
							
							m_oPopupComposite.pack(true);
							controls= m_oPopupComposite.getChildren();
							if(	controls != null &&
								controls.length > 0	)
							{
								pop= m_oPopupComposite.getChildren()[0].getBounds();
							}else
							{
								pop= m_oPopupComposite.getBounds();
							}
							size= m_shellForm.getBounds();
							while(size.height == 0)
							{
								System.out.println("-----  wait for mainwindow  ----------");
								try{
									Thread.sleep(1000);
								}catch(InterruptedException ex)
								{
								}
								size= m_shellForm.getBounds();
							}
							sashHeight[0]= pop.height + HtmTags.popupPadding * 2;
							if(sashHeight[0] < 0)
								sashHeight[0]= 0;
							sashHeight[1]= size.height - pop.height - HtmTags.mainPadding;
							if(sashHeight[1] < 0)
								sashHeight[1]= 0;
							if(HtmTags.debug)
							{
								System.out.println("size height:   " + size.height);
								System.out.println("popup height:  " + pop.height);
								System.out.println("popup padding: " + HtmTags.popupPadding);
								System.out.println("main padding:  " + HtmTags.mainPadding);
								System.out.println("sash size:     " + sashHeight[0] + "/" + sashHeight[1]);
							}
							m_shellForm.setWeights(sashHeight);
						}
						
						@Override
						public void controlMoved(ControlEvent arg0) {
						
							// nothing to do					
						}
					});
				}
			});
		}
		if(client.secondConnection())
		{
			checker= WidgetChecker.instance(m_aTreeNodes.get(0));
			if(!checker.isAlive())
				checker.start();
		}
		
		String firstActiveSide= "";
		String actFolderBefore;
		
		// search first which first side 
		// on server list is set active
		for(TreeNodes current : m_aTreeNodes)
		{
			firstActiveSide= current.getFirstActiveSidePath("");
			if(!firstActiveSide.equals(""))
				break;
		}
		if(HtmTags.informServerLeafPage)
		{
			// sending first to server that all pages not visible,
			// because it can be the case, when the client will be killed
			// or crashes and it was not on the first page
			// the server sinking he is always on this page
			// and by new starting the client want to display the first page
			// after them, two pages are set and the client get signal
			// that the first and the other is active
			// and maybe he switching always from the first to the second
			for (TreeNodes node : m_aTreeNodes)
				node.sendNotVisible();
		}

		if(!firstActiveSide.equals(""))
		{// if first active side on server found,
		 // set this one visible
			if(m_oAktTreeNode != null)
				m_oAktTreeNode.setInvisible();
			m_oAktTreeNode= null;
			actFolderBefore= m_sAktFolder;
			m_sAktFolder= firstActiveSide;
			if(setCurrentSideVisible(/*inform server by no body*/false))
				return;
			m_sAktFolder= actFolderBefore;
		}
		if(m_oAktTreeNode != null)
		{// when no side on server found,
		 // but client was running before
		 // set the same side (actually inside m_sAktFolder) visible
			m_oAktTreeNode= null;
			if(setCurrentSideVisible(/*inform server*/true))
				return;
		}
		
		// when no side on server found
		// and actually no actual side be set from running client before
		// set first regular side, which can be set visible, to active
		setFirstSide(m_aTreeNodes, "");
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
			if(current.treeDisplay())
			{
				m_sAktFolder= path + current.getName();
				if(setCurrentSideVisible(/*inform server by no body*/false))
					return true;
			}
		}
		for(TreeNodes current : nodes)
		{
			if(setFirstSide(current.getChilds(), path + current.getName() + "/"))
				return true;
		}
		return false;
	}
	/**
	 * initial LayoutLoader for working
	 * 
	 * @param type CREATE, UPDATE or REFRESH layout
	 */
	public void setState(short type)
	{
		stateLock.lock();
		this.type= type;
		newStateCondition.signalAll();
		stateLock.unlock();
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
	private ArrayList<TreeNodes> creatingWidgets(final Tree tree, Composite subComposite, Set<String> folderSet) throws IOException
	{
		boolean access, bNewNode;
		short pos= 0;
		TreeNodes node= null;
		DialogThread dialog= DialogThread.instance(m_oTopLevelShell);
		String folderName, aktName= "";
		ArrayList<String> folder= new ArrayList<String>();
		ArrayList<String> aktFolderList= new ArrayList<String>();
		ArrayList<TreeNodes> oRetTrees= new ArrayList<TreeNodes>();
		RE nameParser= new RE("^(.*)\\." + TreeNodes.m_sLayoutStyle);
		String display;
		HashMap<String, String> mMetaBlock;

		TreeNodes.m_nReadCount= 0;
		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			return oRetTrees;
		if(folderSet.size() > 0)
			dialog.setSteps(folderSet.size() * 2);
		else
			dialog.setSteps(1);
		dialog.setSelection((short)0);
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
					bNewNode= true;
					if(m_aTreeNodes != null)
					{
						for (TreeNodes treeNode : m_aTreeNodes)
						{
							if(	treeNode.getName().equals(aktName)	)
							{
								bNewNode= false;
								node= treeNode;
								break;
							}
						}
					}
					try{
						if(bNewNode)
						{
							if(HtmTags.notree)
								node= new TreeNodes(pos, m_oPopupComposite, subComposite, tree, aktFolderList);
							else
								node= new TreeNodes(pos, subComposite, tree, aktFolderList);
						}else
							node.createSide(pos, aktFolderList);
						
					}catch(IllegalAccessException ex)
					{
						if(ex.getMessage().equals("no side access"))
						{
							TreeNodes.m_nReadCount+= (aktFolderList.size() - 1);
							if(HtmTags.debug)
								System.out.println("user has no access to side!");
						}
						access= false;
					}
					if(access)
					{
						oRetTrees.add(node);
						display= null;
						mMetaBlock= node.getMetaData();
						if(mMetaBlock != null)
							display= mMetaBlock.get("display");
						if(	HtmTags.showFalse ||
							display == null ||
							!display.equals("notree")	)
						{// do not increment position index for navigation tree
							++pos; // when the side should'nt display in tree
						}
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
		bNewNode= true;
		if(m_aTreeNodes != null)
		{
			for (TreeNodes treeNode : m_aTreeNodes)
			{
				if(	treeNode.getName().equals(aktName)	)
				{
					bNewNode= false;
					node= treeNode;
					break;
				}
			}
		}
		try{
			if(bNewNode)
			{
				if(HtmTags.notree)
					node= new TreeNodes(pos, m_oPopupComposite, subComposite, tree, aktFolderList);
				else
					node= new TreeNodes(pos, subComposite, tree, aktFolderList);
			}else
				node.createSide(pos, aktFolderList);
			
		}catch(IllegalAccessException ex)
		{
			if(ex.getMessage().equals("no side access"))
			{
				TreeNodes.m_nReadCount+= (aktFolderList.size() - 1);
				if(HtmTags.debug)
					System.out.println("user has no access to side!");
			}
			access= false;
		}
		if(access)
		{
			oRetTrees.add(node);
			display= null;
			mMetaBlock= node.getMetaData();
			if(mMetaBlock != null)
				display= mMetaBlock.get("display");
			if(	HtmTags.showFalse ||
				display == null ||
				!display.equals("notree")	)
			{// do not increment position index for navigation tree
				++pos; // when the side should'nt display in tree
			}
		}
		dialog.setSelection((short)100);
		
		return oRetTrees;			
	}
}
