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

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.apache.regexp.RE;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.ScrolledComposite;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeItem;

import at.kolli.dialogs.DialogThread;
import at.kolli.dialogs.DisplayAdapter;
import at.kolli.layout.Body;
import at.kolli.layout.Break;
import at.kolli.layout.FontObject;
import at.kolli.layout.Head;
import at.kolli.layout.HtmTags;
import at.kolli.layout.IComponentListener;
import at.kolli.layout.Layout;
import at.kolli.layout.PopupMenu;
import at.kolli.layout.Title;
import at.kolli.layout.XMLSaxParser;
import at.kolli.layout.permission;

/**
 * Node for every subroutine
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 30.11.2007
 * @since JDK 1.6
 */
public class TreeNodes
{
	/**
	 * static locker for display actions
	 */
	public static Boolean m_DISPLAYLOCK= true;
	/**
	 * whether node is selected and visible on the display
	 */
	private boolean m_bVisible= false;
	/**
	 * position inside node tree
	 */
	private short m_nPosition;
	/**
	 * count of exist tree nodes
	 * set by ending
	 */
	public static int m_nNodeCount= 0;
	/**
	 * current disposed tree nodes
	 */
	private static int m_nDisposedNodes= 0;
	/**
	 * synchronizing object for boolean of m_bSideCreatged
	 */
	private Object m_CREATEDOBJ= new Object();
	/**
	 * whether side components was created
	 */
	private Boolean m_bSideCreated= false;
	/**
	 * name of parent node
	 */
	private String m_sParentFolder;
	/**
	 * name of node
	 */
	private String m_sName= "";
	/**
	 * displayed name of node on the screen
	 */
	private String m_sTitleName= "";
	/**
	 * parent TreeNodes if node is not in root directory
	 */
	private TreeNodes m_oParentNode;
	/**
	 * subnodes of node, if the node is also an folder
	 */
	private ArrayList<TreeNodes> m_aSubnodes;
	/**
	 * parent Tree if item is in the root
	 */
	private Tree m_oParent= null;
	/**
	 * item of the tree which is displayed
	 */
	private TreeItem m_oItem= null;
	/**
	 * main Composite for every node with StackLayout for scrolling
	 */
	private ScrolledComposite m_oScrolledComposite= null;
	/**
	 * first composite in scollable main composite
	 */
	private Composite m_oComposite= null;
	/**
	 * main composite where setting the StackLayout, coming from outside
	 */
	private Composite m_oSubComposite;
	/**
	 * pop up composite if settings for no tree
	 */
	private Composite m_oPopupComposite= null;
	/**
	 * Body-tag with user-layout (tables) and all components
	 */
	private Body m_oBodyTag= null;
	/**
	 * instance variable for throwing exception inside DisplayAdapter.syncExec
	 */
	private IOException m_runnable_ex;
	/**
	 * holder from all components to get an faster access for actions
	 */
	private ArrayList<IComponentListener> m_aoButtons;
	/**
	 * comprised all meta data witch are defined in the header of the side
	 */
	private HashMap<String, String> m_mMetaBlock= null;
	/**
	 * definition of classes inside XML layout file
	 */
	private HashMap<String, HtmTags> m_mClassDefinitions= null;
	/**
	 * boolean whether can save layout files localy
	 */
	public static boolean m_bSaveLoacal= true;
	/**
	 * path position where home directory for user output
	 */
	public static String m_sHomePath;
	/**
	 * dialog settings for specific user account
	 */
	public static IDialogSettings m_Settings= null;
	/**
	 * path position where layout files are localy saved
	 */
	public static String m_sLayoutPath= "";
	/**
	 * want to read for an normally desktop application string is 'desctop',
	 * otherwise for an touchpad application string is 'touchpad' 
	 */
	public static String m_sLayoutStyle;
	/**
	 * binding client with this user name to server
	 */
	public static String m_sUser= "";
	/**
	 * binding client with this password to server
	 */
	public static String m_sPwd= "";
	/**
	 * HashMap form String/Date values of directory from layout-files by server
	 */
	public static HashMap<String, Date> m_hmDirectory= null;
	/**
	 * count of sides which has read
	 */
	public static int m_nReadCount= 0;
	
	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.
	 *
	 * @param pos position of tree entry
	 * @param subComposite main-composite of node in StackLayout
	 * @param parent a tree control which will be the parent of the new instance (cannot be null)
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @throws IllegalAccessException if side have no access for user
	 */
	public TreeNodes(short pos, Composite subComposite, Tree parent, ArrayList<String> folder) 
	throws IllegalAccessException, IOException
	{
		m_oParent= parent;
		init(pos, subComposite, folder, "");
	}

	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.
	 *
	 * @param pos position of tree entry
	 * @param popup for popup main menu when settings for no tree
	 * @param subComposite main-composite of node in StackLayout
	 * @param parent a tree control which will be the parent of the new instance (cannot be null)
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @throws IllegalAccessException if side have no access for user
	 */
	public TreeNodes(short pos, Composite popup, Composite subComposite, Tree parent, ArrayList<String> folder) 
	throws IllegalAccessException, IOException
	{
		m_oParent= parent;
		m_oPopupComposite= popup;
		init(pos, subComposite, folder, "");
	}
		
	/**
	 * load dialog settings on user home directory
	 * 
	 * @return whether file be loaded
	 */
	public static Boolean loadSettings()
	{
		File settings= new File(m_sHomePath + File.separator + "settings.xml");
		
		if(!settings.isFile())
			return false;
		try
		{
			m_Settings.load(settings.toString());
		}catch(IOException ex)
		{
			ex.printStackTrace();
			return false;
		}
		return true;
	}
	
	/**
	 * save dialog settings on user home directory
	 * 
	 * @return whether file be saved correctly
	 */
	public static Boolean saveSettings()
	{
		File settings= new File(m_sHomePath + File.separator + "settings.xml");
		
		try
		{
			m_Settings.save(settings.toString());
			
		}catch(IOException ex)
		{
			ex.printStackTrace();
			return false;
		}
		return true;
	}
	
	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.
	 *
	 * @param subComposite main-composite of node in StackLayout
	 * @param parent a tree control which will be the parent of the new instance (cannot be null)
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @param parentFolder name of parent folder
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	/*public TreeNodes(Composite subComposite, Tree parent, ArrayList<String> folder, String parentFolder)
	{
		if(!HtmTags.notree)
			m_oItem= new TreeItem(parent, SWT.NULL);
		init(subComposite, folder, parentFolder);
	}*/

	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.
	 * 
	 * @param subComposite main-composite of node in StackLayout
	 * @param parentItem a tree control which will be the parent of the new instance (cannot be null)
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	/*public TreeNodes(Composite subComposite, TreeNodes parentItem, ArrayList<String> folder)
	{
		if(!HtmTags.notree)
			m_oItem= new TreeItem(parentItem.getTreeItem(), SWT.NULL);
		init(subComposite, folder, "");
	}*/
	
	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.
	 * 
	 * @param pos position of tree entry
	 * @param subComposite main-composite of node in StackLayout
	 * @param parentItem a tree control which will be the parent of the new instance (cannot be null)
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @param parentFolder name of parent folder
	 * @throws IllegalAccessException if side have no access for user
	 */
	public TreeNodes(short pos, Composite subComposite, TreeNodes parentItem, ArrayList<String> folder, String parentFolder) 
	throws IllegalAccessException, IOException
	{
		m_oParentNode= parentItem;
		init(pos, subComposite, folder, parentFolder);
	}

	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.<br /><br />
	 * 
	 * @param subComposite main-composite of node in StackLayout
	 * @param parent a tree control which will be the parent of the new instance (cannot be null)
	 * @param index the zero-relative index to store the receiver in its parent
	 * @param parentFolder name of parent folder
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	/*public TreeNodes(Composite subComposite, Tree parent, ArrayList<String> folder, int index, String parentFolder) 
	{
		if(!HtmTags.notree)
			m_oItem= new TreeItem(parent, SWT.NULL, index);
		init(subComposite, folder, parentFolder);
	}*/

	/**
	 * Constructs a new instance of this class given its parent (which must be a Tree, TreeItem, or a TreeNodes) 
	 * The item is added to the end of the items maintained by its parent.<br /><br />
	 * 
	 * @param subComposite main-composite of node in StackLayout
	 * @param parentItem a tree control which will be the parent of the new instance (cannot be null)
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @param parentFolder name of parent folder
	 * @param index the zero-relative index to store the receiver in its parent
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	/*public TreeNodes(Composite subComposite, TreeNodes parentItem, ArrayList<String> folder, String parentFolder, int index) 
	{
		if(!HtmTags.notree)
			m_oItem= new TreeItem(parentItem.getTreeItem(), SWT.NULL, index);
		init(subComposite, folder, parentFolder);
	}*/

	/**
	 * returns the name of the node
	 * 
	 * @return real name of node
	 */
	public String getName()
	{
		return m_sName;
	}
	/**
	 * method return all child nodes from this node
	 * 
	 * @return child nodes
	 */
	public ArrayList<TreeNodes> getChilds()
	{
		return m_aSubnodes;
	}
	/**
	 * return the TreeItem of node
	 * 
	 * @return TreeItem
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	public TreeItem getTreeItem()
	{
		return m_oItem;
	}
	/**
	 * return title name of node
	 * 
	 * @return name of node
	 */
	public String getTitle()
	{
		return m_sTitleName;
	}
	/**
	 * get first active side on server when side has meta-tag pageset
	 * and result path is active
	 * 
	 * @param folder actually folder path
	 * @return first founded side
	 */
	public String getFirstActiveSidePath(String folder)
	{
		String sRv= "";
		String result;
		Double res;
		
		result= m_mMetaBlock.get("pageset");						
		if(	result != null &&
			!result.equals("")	)
		{
			MsgClientConnector client= MsgClientConnector.instance();
			
			try{
				res= client.getValue(result, /*throw*/true);
				if(	res != null &&
					res > 0)
				{
					if(!folder.equals(""))
						folder+= ":";
					sRv= folder + getName();
				}
			}catch(IOException ex)
			{
				return "";
			}
		}
		if(sRv.equals(""))
		{
			if(!folder.equals(""))
				folder+= ":";
			for(TreeNodes node : m_aSubnodes)
			{
				sRv= node.getFirstActiveSidePath(folder + getName());
				if(!sRv.equals(""))
					break;
			}
		}
		return sRv;
	}
	/**
	 * set the composite from this node visible in the StackLayout
	 * only if the given folder is the same as the own.
	 * Otherwise pass to the child nodes
	 * 
	 * @param layout Stacklayout which gets the main-composite
	 * @param folder name of folder which should be the same to set visible
	 * @param inform inform server for activating also when node has no body
	 * @return node which is now set on the top
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	public TreeNodes setVisible(final StackLayout layout, String folder, boolean inform)
	{
		boolean bSideCreated;
		String name;
		StringTokenizer token= new StringTokenizer(folder, "/");
		TreeNodes oRv= null;

		if(!token.hasMoreElements())
		{
			if(m_sTitleName.equals("")) // this case should only be when client
				name= "";				// get no sides from server
			else
				return null;
		}else
			name= token.nextToken();
		if(!m_sName.equals(name))
			return null;
		if(	m_mMetaBlock != null &&
			(	inform == true ||
				hasBody()			)	)
		{// inform server also when side has no body
			String result;
			
			result= m_mMetaBlock.get("pageset");						
			if(	result != null &&
				!result.equals("")	)
			{
				MsgClientConnector client= MsgClientConnector.instance();
				
				try{
					client.setValue(result, 1, /*throw*/false);
				}catch(IOException ex)
				{
					// this will not been thrown
				}
			}
		}
		if(	!hasBody()
			&&
			!token.hasMoreElements()	)
		{
			return null;
		}
		synchronized (m_CREATEDOBJ) {
			bSideCreated= m_bSideCreated;
		}
		if(!bSideCreated)
		{
			String sThreadName;
			Thread runThread;
			
			runThread= Thread.currentThread();
			sThreadName= runThread.getName();
			if(sThreadName.equals("LayoutLoader"))
			{
				try{
					if(!createPage())
						return null;
				}catch(IOException ex)
				{
					System.out.println("IOException");
					ex.printStackTrace();
					return null;
				}
			}
		}
		if(!token.hasMoreElements())
		{
			layout.topControl= m_oScrolledComposite;
			DisplayAdapter.syncExec(new Runnable() {
			
				public void run() {

					m_oSubComposite.layout();
					m_oScrolledComposite.setFocus();
				}
			
			});
			m_bVisible= true;
			return this;
		}
		for(TreeNodes node : m_aSubnodes)
		{
			oRv= node.setVisible(layout, folder.substring(name.length()+1), inform);
			if(oRv != null)
				return oRv;
		}
		return null;
	}
	/**
	 * sending to all sides set on server that side is not visible
	 */
	public void sendNotVisible()
	{
		MsgClientConnector client= MsgClientConnector.instance();

		try{
			if(m_mMetaBlock != null)
			{
				String result;
				
				result= m_mMetaBlock.get("pageset");						
				if(	result != null &&
					!result.equals("")	)
				{
					client.setValue(result, 0, /*throw*/false);
				}
			}
		}catch(IOException ex)
		{}
		for(TreeNodes node : m_aSubnodes)
			node.sendNotVisible();
	}
	/**
	 * set this node and all child nodes to invisible
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	public void setInvisible()
	{
		MsgClientConnector client= MsgClientConnector.instance();
		
		m_bVisible= false;
		try{
			client.clearHearing(/*bthrow*/false);
			if(	m_mMetaBlock != null &&
				HtmTags.informServerLeafPage	)
			{
				String result;
				
				result= m_mMetaBlock.get("pageset");						
				if(	result != null &&
					!result.equals("")	)
				{
					client.setValue(result, 0, /*throw*/false);
				}
			}
		}catch(IOException ex)
		{}
    	if(	HtmTags.debug &&
    		client.hasError()	)
    	{
    		System.out.println("ERROR: by clear all hearing subroutines in TreeNodes.setInvisible()");
    		System.out.println("       " + client.getErrorMessage());
    	}
		for(TreeNodes node : m_aSubnodes)
			node.setInvisible();
	}
	
	/**
	 * returns true if the node is visible on the screen
	 * 
	 * @return true if the node is visible, otherwise false
	 * @author Alexander Kolli
	 * @version 1.00.00, 02.12.2007
	 * @since JDK 1.6
	 */
	public boolean isVisible()
	{
		return m_bVisible;
	}
	
	/**
	 * count tree node with sub-nodes
	 * 
	 * @return count of tree nodes
	 */
	public int count()
	{
		int nRv= 1;

		for (TreeNodes node : m_aSubnodes) {
			
			nRv+= node.count();
		}
		return nRv;
	}
	
	/**
	 * Disposes of the operating system resources associated with the receiver and all its descendants.<br />
	 * This method dispose also recursively all sides of subtrees
	 * 
	 * @param subnodes whether should also dispose sub-nodes
	 */
	public void dispose(boolean subnodes)
	{
		DialogThread dialog= null;
		
		if(LayoutLoader.instance().getType() == LayoutLoader.DISPOSE)
		{
			String sSideCount;
			dialog= DialogThread.instance();
			MsgTranslator trans= MsgTranslator.instance();
			
			++m_nDisposedNodes;
			sSideCount= "" + m_nDisposedNodes + "/" + m_nNodeCount;
			dialog.show(trans.translate("dialogDisposeSide", getTitle(), sSideCount));
			
		}
		PopupMenu.destroy(this);
		DisplayAdapter.syncExec(new Runnable() {
		
			public void run() {

				m_oScrolledComposite.dispose();
			}
		
		});
		if(LayoutLoader.instance().getType() == LayoutLoader.DISPOSE)
			dialog.nextStep();

		if(subnodes)
		{
			for (TreeNodes node : m_aSubnodes) {
				
				node.dispose(/*subnodes*/true);
			}
		}
	}
	/**
	 * method returning meta-data which defined in header from layout file
	 * 
	 * @return meta data
	 */
	public HashMap<String, String> getMetaData()
	{
		return m_mMetaBlock;
	}
	/**
	 * check whether given side path is from current side
	 * 
	 * @param sides path of side
	 * @return whether given sequence is same
	 */
	public boolean isCurrentSidePath(String sides)
	{
		String[] aSides= sides.split("/");
		
		return isCurrentSidePath(aSides);
	}
	/**
	 * check whether given side path is from current side
	 * 
	 * @param sides path of side
	 * @return whether given sequence is same
	 */
	public boolean isCurrentSidePath(String[] sides)
	{
		int nLastPos= sides.length-1;
		
		while(sides[nLastPos].equals(""))
			--nLastPos;
		if(nLastPos < 0)
			return true;
		if(!getName().equals(sides[nLastPos]))
			return false;
		if(nLastPos == 0)
			return true;
		if(m_oParentNode == null)
			return false;
		sides[nLastPos]= "";
		return m_oParentNode.isCurrentSidePath(sides);
	}
	/**
	 * get position inside tree node
	 * 
	 * @return position
	 */
	public short getPosition()
	{
		return m_nPosition;
	}
	/**
	 * initial the node from the constructor and creates all child nodes
	 * 
	 * @param pos position of tree entry
	 * @param subComposite main-composite of node in StackLayout
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @param parentFolder name of parent folder
	 * 
	 * @throws IOException when connection to server is broken or closed
	 * @throws IllegalAccessException if side have no access for user
	 */
	private void init(final short pos, final Composite subComposite, ArrayList<String> folder, String parentFolder) 
	throws IllegalAccessException, IOException
	{ 
		m_mMetaBlock= null;
		m_sParentFolder= parentFolder;
		m_oSubComposite= subComposite;
		m_aSubnodes= new ArrayList<TreeNodes>();
		createSide(pos, folder);
	}
	/**
	 * iterate over all sides from folder and create content when not exist or new
	 * and create also child nodes
	 * 
	 * @param pos position of tree entry
	 * @param subComposite main-composite of node in StackLayout
	 * @param folder array of exist node-names (folder:subroutine:...)
	 * @param parentFolder name of parent folder
	 * 
	 * @throws IOException when connection to server is broken or closed
	 * @throws IllegalAccessException if side have no access for user
	 */
	public void createSide(final short pos, ArrayList<String> folder) 
			throws IllegalAccessException, IOException
	{
		boolean bNoSides= false;
		short ipos= 0;
		String name, display= null;
		String aktName= "";
		ArrayList<String> aktFolderList= new ArrayList<String>();
		DialogThread dialog= DialogThread.instance(null);
		TreeNodes node= null;
		ArrayList<TreeNodes> nodes= new ArrayList<TreeNodes>();

		m_nPosition= pos;
		m_sName= "";
		if(folder.isEmpty())
		{// when TreeNodes started with no content of files in folder
		 // client get no sides from server, so create null page
			folder.add("dummyName");
			bNoSides= true;
		}
		for(String f : folder)
		{
			StringTokenizer token= new StringTokenizer(f, "/");
			//ScrolledComposite oScComp;
			
			name= token.nextToken();
			if(m_sName.isEmpty())
			{
				if(!bNoSides)
				{
					m_sName= name;
					if(m_sTitleName.isEmpty())
						m_sTitleName= name.trim();
				}
				if(dialog.dialogState().equals(DialogThread.states.CANCEL))
					throw new IllegalAccessException("### loading dialog closed");
				
				createContent();
			}else
			{
				if(!m_sName.equals(name))
					throw new InstantiationError("Folder '"+m_sParentFolder+"/"+name+"' is no part of '"+m_sParentFolder+":"+m_sName+"'");
			}
			if(token.hasMoreElements())
			{
				int nLen= name.length();
				String sFolderString= f.substring(nLen+1);
				String nextName= token.nextToken();
				
				if(!aktName.equals(nextName))
				{
					if(!aktFolderList.isEmpty())
					{
						boolean access= true;
						boolean bNewNode= true;
						String p= m_sParentFolder;

						if(!p.isEmpty())
							p+= "/";
						p+= m_sName;
						for (TreeNodes treeNode : m_aSubnodes)
						{
							if(treeNode.getName().equals(aktName))
							{
								bNewNode= false;
								node= treeNode;
								break;
							}
						}
						if(dialog.dialogState().equals(DialogThread.states.CANCEL))
							return;
						try{
							if(bNewNode)
								node= new TreeNodes(ipos, m_oSubComposite, this, aktFolderList, p);
							else
								node.createSide(ipos, aktFolderList);
							
						}catch(IllegalAccessException ex)
						{
							if(ex.getMessage().equals("no side access"))
							{
								access= false;
								m_nReadCount+= (aktFolderList.size() - 1);
							}
						}
						if(access)
						{
							nodes.add(node);
							++ipos;
						}
					}
					aktName= nextName;
					aktFolderList.clear();
					aktFolderList.add(sFolderString);
				}else
					aktFolderList.add(sFolderString);
			}
		}
		if(!aktFolderList.isEmpty())
		{
			boolean access= true;
			boolean bNewNode= true;
			String parentFolder= m_sParentFolder;
			
			parentFolder= parentFolder + "/" + m_sName;
			if(parentFolder.startsWith("/"))
				parentFolder= parentFolder.substring(1);
			for (TreeNodes treeNode : m_aSubnodes)
			{
				if(treeNode.getName().equals(aktName))
				{
					bNewNode= false;
					node= treeNode;
					break;
				}
			}
			if(dialog.dialogState().equals(DialogThread.states.CANCEL))
				return;
			try{
				if(bNewNode)
					node= new TreeNodes(ipos, m_oSubComposite, this, aktFolderList, parentFolder);
				else
					node.createSide(ipos, aktFolderList);
				
			}catch(IllegalAccessException ex)
			{
				if(ex.getMessage().equals("no side access"))
				{
					access= false;
					m_nReadCount+= (aktFolderList.size() - 1);
				}
			}
			if(access)
				nodes.add(node);
		}
		for (TreeNodes onode : m_aSubnodes) 
		{
			boolean found= false;
			
			for (TreeNodes nnode : nodes)
			{
				if(onode == nnode)
				{
					found= true;
					break;
				}
			}
			if(!found)
				onode.dispose(/*subnodes*/true);
		}
		m_aSubnodes= nodes;
		
		if(!hasContent())
		{
			display= null;
			
			if(m_mMetaBlock != null)
				display= m_mMetaBlock.get("display");
			if(	display == null ||
				!display.equals("true")	)
			{
				if(HtmTags.debug)
					System.out.println("Page has no body or child page with body!");
				PopupMenu.destroy(this);
				throw new IllegalAccessException("no side access");
			}
		}
		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			throw new IllegalAccessException("### loading dialog closed");
	}
	/**
	 * check or remove check inside tree
	 * 
	 * @param checked whether tree item should be checked
	 */
	public void check(final boolean checked)
	{
		if(	!HtmTags.notree &&
			m_oItem != null		)
		{
			DisplayAdapter.syncExec(new Runnable() {
			
				public void run() 
				{	
					org.eclipse.swt.graphics.Color curColor;
					
					if(checked)
						curColor= m_oItem.getDisplay().getSystemColor(SWT.COLOR_BLUE);
					else
						curColor= m_oItem.getDisplay().getSystemColor(SWT.COLOR_WHITE);
					m_oItem.setBackground(curColor);
				}			
			});
		}
	}
	/**
	 * read xml layout from file
	 * 
	 * @throws IOException when connection to server is broken or closed
	 * @throws IllegalAccessException if side have no access for user
	 */
	public void createContent()
			throws IOException, IllegalAccessException
	{
		boolean bNoSides= false;
		boolean bWasSideCreated;
		String sName= "", sTitle;
		String sSideCount= "", display= null;
		DialogThread dialog= DialogThread.instance(null);
		MsgTranslator trans= null;
		final GridLayout grid= new GridLayout();
		
		if(HtmTags.useBackgroundLoadingPriority == 0)
		{
			++m_nReadCount;
			sSideCount= "" + m_nReadCount + "/" + m_hmDirectory.size();
			//if(HtmTags.debug)
				System.out.println("create side of '"+getName()+"'");
			trans= MsgTranslator.instance();
			sName= getTitle();
			dialog.show(trans.translate("dialogCreateSide", sSideCount, sName));
			dialog.setSelection((short)0);
		}
		if(!readFile())
		{
			String msg;
			
			if(dialog.isOpen())
			{
				msg= "no side access";
			}else
				msg= "### loading dialog closed";
			throw new IllegalAccessException(msg);
		}
		synchronized (m_CREATEDOBJ) 
	    {
			bWasSideCreated= m_bSideCreated;
		    if(!m_bSideCreated)
		    {		    	
				DisplayAdapter.syncExec(new Runnable() {
					
					public void run() {
						
						m_oScrolledComposite= new ScrolledComposite(m_oSubComposite, SWT.H_SCROLL | SWT.V_SCROLL);
						m_oComposite= new Composite(m_oScrolledComposite, SWT.SHADOW_NONE);
						m_oComposite.setBackground(HtmTags.systemColor);
						m_oScrolledComposite.setBackground(HtmTags.systemColor);
						m_oScrolledComposite.setContent(m_oComposite);
						m_oScrolledComposite.setExpandHorizontal(true);
						m_oScrolledComposite.setExpandVertical(true);
						m_oComposite.setLayout(grid);
					}
				
				});
		    }
	    }
		if(HtmTags.useBackgroundLoadingPriority == 0)
		{
			sTitle= getTitle();
			if(!sTitle.equals(sName))
				dialog.show(trans.translate("dialogCreateSide", sSideCount, sTitle));
			dialog.setSelection((short)0);
			dialog.setSteps(m_aoButtons.size() + 1);
			dialog.nextStep();
			if(!createPage()	)
			{
				String msg;
				
				if(dialog.isOpen())
				{
					dispose(/*subnodes*/false);
					msg= "no side access";
				}else
					msg= "### loading dialog closed";
				throw new IllegalAccessException(msg);
			}
			dialog.setSelection((short)100);
		}
		if(m_mMetaBlock != null)
			display= m_mMetaBlock.get("display");
		if(	!bNoSides &&
			(	display == null ||
				!display.equals("notree")	)	||
			HtmTags.showFalse						)
		{
			if(HtmTags.notree)
			{
				if(	m_oPopupComposite != null &&
					!bWasSideCreated				)
				{
					final TreeNodes tnode= this;
					if(dialog.dialogState().equals(DialogThread.states.CANCEL))
						throw new IllegalAccessException("### loading dialog closed");
					DisplayAdapter.syncExec(new Runnable() {
					
						public void run() 
						{
							PopupMenu.init(m_oPopupComposite, tnode);
						}
					
					});
				}
			}else
			{
				if(dialog.dialogState().equals(DialogThread.states.CANCEL))
					throw new IllegalAccessException("### loading dialog closed");
				DisplayAdapter.syncExec(new Runnable() {
				
					public void run() {

						boolean inner= false;
						TreeItem[] items;
						
						if(m_oParent != null)
							items= m_oParent.getItems();
						else
							items= m_oParentNode.getTreeItem().getItems();

						for(short c= m_nPosition; c < items.length; c++)
						{
							if(items[c].getText().equals(m_sTitleName))
							{
								inner= true;
								m_oItem= items[c];
								break;
							}else
								items[c].dispose();
						}
						if(!inner)
						{
							if(m_oParent != null)
								m_oItem= new TreeItem(m_oParent, SWT.NULL);
							else
								m_oItem= new TreeItem(m_oParentNode.getTreeItem(), SWT.NULL);
							m_oItem.setText(m_sTitleName);
						} 
				
					}
				
				});
			}
		}else if(	display != null &&
					display.equals("notree")	)
		{
			String pageset= null;
			
			if(m_mMetaBlock != null)
				pageset= m_mMetaBlock.get("pageset");
			if(pageset == null)
			{
				System.out.println("WARNING: page " + m_sName + " is not shown inside navigation tree and has not pageset from server,");
				System.out.println("         so page can not be reached in any case");
			}
		}

		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			throw new IllegalAccessException("### loading dialog closed");
		dialog.nextStep();
		//oScComp.setContent(m_oContent);
		//m_oContent.setSize(m_oContent.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	/**
	 * read XML side witch get from server 
	 * 
	 * @return whether the side can displayed
	 */
	private boolean readFile() throws IOException
	{
		boolean exists= false;
		boolean writeFile= true;
		String permGroup;
		InputStream emptyStream= null;
		Layout layout= null;
		Head head= null;
		Title title;
		String sTitle, display;
		String fileName= m_sParentFolder+"/"+m_sName;
		String osFileName;
		File file= null;
		SAXParserFactory factory= null;
		XMLSaxParser handler= null;
		MsgClientConnector client= MsgClientConnector.instance();
		ArrayList<Body> bodyList= null;
		Date serverDate;
		Date fileDate;
		DialogThread dialog;
		RE simpleError= new RE("^[ \t]*ERROR");

		if(m_sName.equals(""))
		{// when TreeNodes started with no content of files (m_sName is "")
		 // client get no sides from server, so create null page
			String noFiles;
			String msg= "ERROR: get no files with extension ." + m_sLayoutStyle + " from server";
			
			noFiles=  "<layout>";
			noFiles+=   "<body>";
			noFiles+=     msg;; 
			noFiles+=   "</body>";
			noFiles+= "</layout>";
			emptyStream= new ByteArrayInputStream(noFiles.getBytes());
			if(HtmTags.debug)
				System.out.println(msg);
			
		}else
		{
			if(fileName.startsWith("/"))
				fileName= fileName.substring(1);
			if(File.separator.equals("/"))
				osFileName= fileName;
			else
				osFileName= fileName.replace("/", File.separator);
			file= new File(m_sLayoutPath + File.separator + osFileName+"." + m_sLayoutStyle );
			
			if(HtmTags.debug)
				System.out.println("read file: '" + file + "'");
			
			serverDate= m_hmDirectory.get(fileName + "." + m_sLayoutStyle);
			if(serverDate == null)
			{
				String emptyFile;
				// if found an locally layout file but the date for server do not exist
				// this case can be when searching an layout for an sub-directory
				// but on server is no extra file for this directory
				// now create an own empty one
				emptyFile=	"<layout>";
				emptyFile+=	"</layout>";
				//emptySource= new InputSource(emptyFile);
				emptyStream= new ByteArrayInputStream(emptyFile.getBytes());
				if(HtmTags.debug)
					System.out.println("no file is available, create an empty file with no body");
				
			}else if(file.isFile())
			{
				fileDate= new Date(file.lastModified());
				if(fileDate.after(serverDate))
				{
					synchronized (m_CREATEDOBJ)
					{
						if(m_bSideCreated)
							return true;
					}
					exists= true;
					if(!file.canRead())
					{
						System.out.println("cannot read file "+fileName);
						exists= false;
						
					}else if(HtmTags.debug)
						System.out.println("take local stored file");
				}
			}
			if( emptyStream == null &&
				!exists					)
			{
				FileOutputStream output;
				ArrayList<String> xmlFile;
				File path= file.getParentFile();
				
				if(HtmTags.debug)
					System.out.println("take file from server");
				xmlFile= client.getContent(fileName + "." + m_sLayoutStyle, /*bthrow*/true);
				if(	xmlFile == null ||
					xmlFile.size() == 0	)
				{
					if(HtmTags.debug)
					{
						System.out.println("cannot read file '" + fileName + "." + m_sLayoutStyle + "' from server");
						System.out.println(client.getErrorMessage());
					}
					return false;
					
				}else if(	xmlFile.size() == 1 &&
							simpleError.match(xmlFile.get(0))	)
				{
					StringBuffer side= new StringBuffer();
					
					writeFile= false;
					side.append("<layout>");
					side.append("  <body>");
					side.append(client.getErrorMessage());
					side.append("<br />'");
					side.append(fileName);
					side.append(".");
					side.append(m_sLayoutStyle);
					side.append("'");
					side.append("  </body>");
					side.append("</layout>");	
					emptyStream= new ByteArrayInputStream(side.toString().getBytes());
				}
				
				if(writeFile)
				{
					if(!path.isDirectory())
					{
						path.mkdirs();
					}
					try{
						output= new FileOutputStream(file);
						for(String row : xmlFile)
						{
							row+= "\n";
							output.write(row.getBytes());
						}
						output.close();
					}catch(IOException ex)
					{
						System.out.println(ex);
					}
				}
			}
		}
		synchronized (m_CREATEDOBJ) {
			if(m_bSideCreated)
			{
				dispose(/*subnodes*/false);
				m_bSideCreated= false;
			}
		}
		dialog= DialogThread.instance();
		if(dialog.dialogState().equals(DialogThread.states.CANCEL))
			return false;
		
	    try {
	        // Use an instance of ourselves as the SAX event handler
	        handler = new XMLSaxParser(fileName);
	        // Parse the input with the default (non-validating) parser
	        factory= SAXParserFactory.newInstance();
	        factory.setValidating(true);
	        SAXParser saxParser = factory.newSAXParser();
	        if(emptyStream == null)
	        	saxParser.parse(file, handler );
	        else
	        	saxParser.parse(emptyStream, handler);
	        m_mMetaBlock= handler.getMetaBlock();
	        dialog.nextStep();
	        if(	m_mMetaBlock != null &&
	        	!HtmTags.showFalse		)
	        {
	        	display= m_mMetaBlock.get("display");
	        	
	        	if(	display != null &&
	        		display.equals("false")	)
		        {
		        	if(HtmTags.debug)
		        		System.out.println("page is set to display false, do not show this page inside navigation bar");
		        	return false;
		        }
	        }
	        m_aoButtons= handler.getComponents();
		    layout= (Layout)handler.getMainTag();
		    head= layout.getHead();
		    bodyList= layout.getBody();
	      } catch( Throwable t ) 
	      {
	    	  dialog.nextStep();
	    	  m_mMetaBlock= handler.getMetaBlock();
				if(	m_mMetaBlock != null &&
				    	!HtmTags.showFalse		)
				{
					display= m_mMetaBlock.get("display");
					
					if(	display != null &&
						display.equals("false")	)
				    {
				    	if(HtmTags.debug)
				    		System.out.println("page is set to display false, do not show this page inside navigation bar");
				    	return false;
				    }
				}
	    	  if( t.getMessage() == null ||
	    		  !t.getMessage().equals("no side access")	)
	    	  {
	    		  t.printStackTrace();
	    	  }
	        if(m_mMetaBlock == null)
	        	m_mMetaBlock= new HashMap<String, String>();
	        if(layout == null)
	        {
	        	Body body;
	        	at.kolli.layout.Label label;
	        	
	        	layout= (Layout)handler.getMainTag();
	        	if(layout != null)
	        	{
	    		    head= layout.getHead();
	    		    bodyList= layout.getBody();
	    		    if(	bodyList != null
	    		    	&&
	    		    	bodyList.size() > 0	)
	    		    {
	    		    	body= bodyList.get(0);
	    		    }else
	    		    {
	    		    	bodyList= new ArrayList<Body>();
	    		    	body= new Body();
	    		    	bodyList.add(body);
	    		    }
	        	}else
	        	{
	        		bodyList= new ArrayList<Body>();
	        		layout= new Layout();
	        			body= new Body();
	        			layout.insert(body);
	        		bodyList.add(body);
	        	}
	        	body.insert(new Break());
	        	body.insert(new Break());
	        	label= new at.kolli.layout.Label();	        	
	        	label.setText("### layout ERROR: " + t.getMessage());
	        	body.insert(label);
	        }
	      }
	      permGroup= m_mMetaBlock.get("permission");
	      if(permGroup != null)
	      {
	    	  permission perm;

	    	  if(dialog.dialogState().equals(DialogThread.states.CANCEL))
	    		  return false;
	    	  perm= client.permission(permGroup, /*bthrow*/true);	    		  
	    	  if(	perm == null ||
	    			perm == permission.None	)
	    	  {
	    		  if(HtmTags.debug)
	    		  {
	    				if(client.hasError())
	    					System.out.println(client.getErrorMessage());
	    				else
	    					System.out.println("Client hase no access to group(s) '" + permGroup + "'");
	    		  }
	    		  return false;
	    	  }
	      }
	      sTitle= "";
	      display= null;
		    if(head != null)
		    {
		    	title= head.getTitle();
		    	if(title != null)
		    	{
		    		sTitle= title.name.trim();
		    		if(sTitle == null)
		    			sTitle= "";
		    	}
		    	if(m_mMetaBlock != null)
		    	{
		    		display= m_mMetaBlock.get("display");
		    		if(display != null)
		    		{
		    			display= display.toLowerCase();
		    			if(	!display.equals("true") &&
		    				!display.equals("false") &&
		    				!display.equals("notree")	)
		    			{
		    				System.out.println("WARNING: for meta tag 'display' wrong content with '" + display + "' be set");
		    				display= m_mMetaBlock.get("display");
		    				display= "(?) " + display + " (?)";
		    			}
		    		}
		    	}
		    }
			if(HtmTags.showFalse)
			{
				String sFile= m_sTitleName;
				
				m_sTitleName= "";
				if(display != null)
					m_sTitleName= "[" + display + "]  ";
				if(sTitle.equals(""))
					m_sTitleName+= sFile;
				else
					m_sTitleName+= sTitle;
				m_sTitleName+= "  (" + sFile + ")";
				
			} else if(!sTitle.equals(""))
				m_sTitleName= sTitle;
		    if(	bodyList != null
		    	&&
		    	bodyList.size() > 0	)
		    {
			    m_oBodyTag= bodyList.get(0);
			    if(m_oBodyTag != null)
			    	m_mClassDefinitions= handler.getClassDefinitions();
		    }
	      return true;
	}

	/**
	 * create XML side witch get from server and execute all components
	 * which should be displayed
	 * 
	 * @return whether the side can displayed
	 */
	synchronized public boolean createPage() throws IOException
	{	
		DialogThread dialog;

	    synchronized (m_CREATEDOBJ) {
		    if(m_bSideCreated)
		    	return true;
		}
	    if(m_oBodyTag != null)
	    {	
	    	m_runnable_ex= null;
			dialog= DialogThread.instance();
			if(dialog.dialogState().equals(DialogThread.states.CANCEL))
				return false;
	    	DisplayAdapter.syncExec(new Runnable() {
			
				public void run() {
					
					try{
						GridLayout layout= new GridLayout();

						layout.marginLeft= 0;
						layout.marginRight= 0;
						layout.marginTop= 0;
						layout.marginBottom= 0;
						layout.horizontalSpacing= 0;
						layout.verticalSpacing= 0;
						m_oComposite.setLayout(layout);
						m_oBodyTag.execute(m_oComposite, new FontObject(), m_mClassDefinitions);
					}catch(IOException ex)
					{
						m_runnable_ex= ex;
					}
				}
			
			});
	    	if(m_runnable_ex != null)
	    		throw m_runnable_ex;
			if(m_oScrolledComposite != null)
			{
				DisplayAdapter.syncExec(new Runnable() {
					
					@Override
					public void run() {
						// define minimal shown size
						Point size;
						
						if(HtmTags.debug)
						{
							Control[] childs;
							org.eclipse.swt.widgets.Layout layout;
							
							layout= m_oComposite.getLayout();
							System.out.println("define minimal scroll size for "+layout);
							layout= m_oComposite.getParent().getLayout();
							System.out.println("        with parent "+layout);
							childs= m_oComposite.getChildren();
							for (Control child : childs) {
								layout= ((Composite)child).getLayout();
								System.out.println("              child "+layout);
							}
						}
						size= m_oComposite.computeSize(SWT.DEFAULT, SWT.DEFAULT, true);
						m_oScrolledComposite.setMinSize(size);
						//System.out.println(m_sName + ": " + m_oComposite.getSize());
					}
				});
			}
			dialog.setSelection(dialog.getMaximum());
	    }
	    synchronized (m_CREATEDOBJ) {
		    m_bSideCreated= true;			
		}
	    return true;
	}
	
	/**
	 * show whether node has an body
	 * 
	 * @return true when node has an body, otherwise false
	 */
	public boolean hasBody()
	{
		return m_oBodyTag != null;
	}
	
	/**
	 * show whether node has body or child nodes with body
	 * 
	 * @return true when node has child nodes, otherwise false
	 */
	public boolean hasContent()
	{
		if(hasBody())
			return true;
		for (TreeNodes node : m_aSubnodes)
		{
			if(node.hasContent())
				return true;
		}
		return false;
	}
	
	/**
	 * return an array of all components in this node
	 * 
	 * @return array of components
	 */
	public ArrayList<IComponentListener> getComponents()
	{
		return m_aoButtons;
	}
	/**
	 * server listener if the node is visible,
	 * otherwise function pass to the child nodes
	 * 
	 * @param client interface handle to the server
	 */
	public void listenClient(ClientConnector client, NodeContainer cont) throws IOException
	{
		if(isVisible())
		{
			if(m_aoButtons != null)
			{
				Map<String, Double> results= new HashMap<String, Double>();
				
				for(IComponentListener component : m_aoButtons)
					component.serverListener(results, cont);
			}
			return;
		}
		
		if(m_aSubnodes != null)
		{
			for(TreeNodes node : m_aSubnodes)
				node.listenClient(client, cont);
		}
	}
	/**
	 * search whether folder and subroutine shows to an defined side in meta tags pageset
	 * 
	 * @param sub folder and subroutine separated by an colon
	 * @return new side name when should be activated
	 */
	public String getPageFrom(String sub)
	{
		String sRv, result;
		
		if(m_mMetaBlock != null)
		{
			result= m_mMetaBlock.get("pageset");
			if(	result != null &&
				result.equals(sub)	)
			{
				return m_sName;
			}
		}
		sRv= "";
		if(m_aSubnodes != null)
		{
			for(TreeNodes node : m_aSubnodes)
			{
				sRv= node.getPageFrom(sub);
				if(!sRv.equals(""))
				{
					sRv= m_sName + "/" + sRv;
					break;
				}
			}
		}
		return sRv; 
	}
	/**
	 * whether page should display inside navigation tree
	 * 
	 * @return whether should display
	 */
	public boolean treeDisplay()
	{
		String display= null;
		
		if(HtmTags.showFalse)
			return true;
		if(m_mMetaBlock != null)
			display= m_mMetaBlock.get("display");
		if(	display == null ||
			!display.toLowerCase().equals("notree")	)
		{
			return true;
		}
		return false;
	}
	/**
	 * insert into server connection all sides having an defined address
	 * in meta-tags for pageset
	 * 
	 * @param firstDef whether definition of sides are the first one.<br />
	 *                 to sending command of <code>hear()</code>
	 *                 and not as otherwise <code>holdHearing()</code>
	 */
	public void hearOnSides(boolean firstDef)
	{

		MsgClientConnector client;
		String result;
		
		if(	m_mMetaBlock != null &&
			m_oBodyTag != null		)
		{
			result= m_mMetaBlock.get("pageset");
			if(	result != null &&
				!result.equals("")	)
			{
				client= MsgClientConnector.instance();
				try{
					if(firstDef)
						client.hear(result, /*throw*/false);
					else
						client.holdHearing(result, /*throw*/false);
					
				}catch(IOException ex)
				{
					// this will not been thrown
				}
			}
		}
		for(TreeNodes node : m_aSubnodes)
			node.hearOnSides(firstDef);
	}
}
