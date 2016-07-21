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
package at.kolli.layout;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Shell;

import at.kolli.automation.client.LayoutLoader;
import at.kolli.automation.client.TreeNodes;
import at.kolli.dialogs.DisplayAdapter;
import at.kolli.layout.FontObject.colors;

/**
 * singelton pattern for popup menu if the parameter -t for no tree be set
 * 
 * @author Alexander Kolli
 * @version 1.0
 *
 */
public class PopupMenu 
{
	/**
	 * single instance of popup menu
	 */
	private static PopupMenu _instance= null;
	/**
	 * calculated rectangle of popup-menu after the first display
	 */
	private Rectangle m_nPopup= null;
	/**
	 * shell of current popup menu
	 */
	private Shell m_popupShell= null;
	/**
	 * top level shell from main window
	 */
	private Shell m_oTopLevelShell;
	/**
	 * composite in which should appear the menu bar
	 */
	private Composite m_oMenu;
	/**
	 * root nodes from all read trees
	 */
	private Hashtable<String, TreeNodes> m_mRootNodes= new Hashtable<String, TreeNodes>();
	/**
	 * all main entries in the menu with defined group composite
	 */
	private Hashtable<String, Group> m_mRootEntrys= new Hashtable<String, Group>();
	/**
	 * inherits the name of menu which apear currently on screen.
	 * Otherwise the string is ""
	 */
	private String m_sMenu= "";
	/**
	 * Listener for main pop-up menu
	 */
	private Hashtable<String, HashMap<Control, MouseListener> > m_aMainPopupListeners= new Hashtable<String, HashMap<Control,MouseListener>>();
	/**
	 * Listener for opened pop-up list
	 */
	private HashMap<Control, MouseListener> m_aPopupListeners= new HashMap<Control, MouseListener>();
	
	/**
	 * private destructor to create object of popup menu
	 * 
	 *  @param topShell parent top level shell
	 *  @param menubar composite
	 */
	private PopupMenu(Composite menubar)
	{
		m_oTopLevelShell= menubar.getShell();
		m_oMenu= menubar;
	}
	/**
	 * creating new object of pop-up menu if not exist
	 * and set main menu entry
	 * 
	 * @param menubar composite where the root of menues should appear
	 * @param node object of tree node for pop-up menu
	 */
	public static void init(Composite menubar, TreeNodes node)
	{
		RowLayout layout= new RowLayout();
		final Group popup= new Group(menubar, SWT.NONE);
		Label text= new Label(popup, SWT.SHADOW_IN);
		HashMap<String, String> metablock= node.getMetaData();
		final String entry;
		String sLabel;
		String spacing= metablock.get("popupspace");
		int space;
		FontObject font= new FontObject();
		int popupsize= 0;
		boolean bold= false;
		boolean italic= false;
		String looks, ssize;
		MouseListener listener1, listener2;
		HashMap<Control, MouseListener> ListenerMap= new HashMap<Control, MouseListener>();

		if(_instance == null)
		{
			_instance= new PopupMenu(menubar);
		}

		entry= node.getName();
		sLabel= node.getTitle();
		ssize= metablock.get("popupfontsize");
		if(ssize != null)
			popupsize= Integer.parseInt(ssize);
		looks= metablock.get("popupfontstyle");
		if(looks != null)
		{
			String[] split;
			
			split= looks.split(",");
			for(int i= 0; i < split.length; ++i)
			{
				split[i]= split[i].trim().toLowerCase();
				if(split[i].equals("bold"))
					bold= true;
				else if(split[i].equals("italic"))
					italic= true;
			}
		}
		font.defineColor(popup, metablock.get("popupcolor"), colors.BACKGROUND, entry + " popup");
		font.defineColor(popup, metablock.get("popupfontcolor"), colors.TEXT, entry + " popup");
		font.defineFont(popup, metablock.get("popupfont"), popupsize, bold, italic, /*underline*/false);
		text.setText(sLabel);
		if(spacing != null)
			space= Integer.parseInt(spacing);
		else
			space= 15;
		layout.marginWidth= 0;
		layout.marginHeight= 0;
		layout.marginTop= space;
		layout.marginRight= space + (space /2);
		layout.marginBottom= space;
		layout.marginLeft= space + (space /2);
		popup.setLayout(layout);
		font.setDevice(popup);
		font.setDevice(text);
		font.dispose();
		text.addMouseListener(listener1= new MouseAdapter() 
		{
			public void mouseDown(MouseEvent event)
			{
				LayoutLoader loader;
				Thread t= null;
				
				if(	HtmTags.lockDebug	)
				{
					t= Thread.currentThread();
					System.out.println(t.getName()+" show pop-up menue of " + entry);
				}
				_instance.show(entry, false);
				if(	HtmTags.lockDebug	)
				{
					System.out.println(t.getName()+" want to setActiveSideVisible of " + entry);
				}
				synchronized (TreeNodes.m_DISPLAYLOCK)
				{
					loader= LayoutLoader.instance();
					loader.m_sAktFolder= entry;
					if(HtmTags.lockDebug	)
					{
						System.out.println(t.getName()+" setActiveSideVisible of " + entry);
					}
					loader.setActSideVisible(/*inform server by no body*/true);
					if(	HtmTags.lockDebug	)
					{
						System.out.println(t.getName()+" hase setActiveSideVisible of " + entry);
					}
				}
			}
		});
		popup.addMouseListener(listener2= new MouseAdapter() 
		{
			public void mouseDown(MouseEvent event)
			{
				LayoutLoader loader;
				Thread t= null;
				
				if(HtmTags.lockDebug)
				{
					t= Thread.currentThread();
					System.out.println(t.getName()+" show pop-up menue of " + entry);
				}
				_instance.show(entry, false);
				if(HtmTags.lockDebug)
					System.out.println(t.getName()+" want to setActiveSideVisible of " + entry);
				synchronized (TreeNodes.m_DISPLAYLOCK)
				{
					loader= LayoutLoader.instance();
					loader.m_sAktFolder= entry;
					if(HtmTags.lockDebug)
						System.out.println(t.getName()+" setActiveSideVisible of " + entry);
					loader.setActSideVisible(/*inform server by no body*/true);
					if(HtmTags.lockDebug)
						System.out.println(t.getName()+" hase setActiveSideVisible of " + entry);
				}
			}
		});
		ListenerMap.put(text, listener1);
		ListenerMap.put(popup, listener2);
		_instance.m_aMainPopupListeners.put(entry, ListenerMap);
		_instance.m_mRootEntrys.put(entry, popup);
		_instance.m_mRootNodes.put(entry, node);
	}
	/**
	 * destroy menu entry from pop-up list
	 * 
	 * @param node object of tree node for pop-up menu to destroy
	 */
	public static void destroy(TreeNodes node)
	{
		if(_instance != null)
		{
			final String name= node.getName();

			DisplayAdapter.syncExec(new Runnable() {
			
				public void run() 
				{
					Group popup;
					HashMap<Control, MouseListener> listenerMap;
					

					popup= _instance.m_mRootEntrys.get(name);
					if(popup != null)
					{
						// Alexander Kolli 01/02/2014
						// do not need to remove mouse listeners 
						// when I dispose the widget
						/*listenerMap= _instance.m_aMainPopupListeners.get(name);
						if(	listenerMap != null &&
							!listenerMap.isEmpty()	)
						{
							for(Control key : listenerMap.keySet())
							{
								MouseListener listener;
								
								listener= listenerMap.get(key);					
								if(	listener != null	)
								{
									key.removeMouseListener(listener);
									listenerMap.remove(key);
								}
							}	
						}*/
						popup.dispose();
					}
				}
		
			});
			_instance.m_aMainPopupListeners.remove(name);
			_instance.m_mRootEntrys.remove(name);
			_instance.m_mRootNodes.remove(name);
		}
	}
	/**
	 * clearing all menu entries from pop-up list
	 * 
	 */
	public static void clearAll()
	{
		if(_instance != null)
		{
			_instance.destroyPopupShell();
			_instance.m_mRootEntrys.clear();
			_instance.m_mRootNodes.clear();
		}
		_instance= null;
	}
	/**
	 * returning instance of this object if exit
	 */
	public static PopupMenu instance()
	{
		return _instance;
	}
	/**
	 * show one popup menu over the main window
	 * 
	 * @param menu_entry actual entry which should be shown or disposed
	 * @param move TRUE if the curser be moved over one entry, otherwise by clicking false
	 */
	private void show(String menu_entry, boolean move)
	{
		Point abs;
		Group popup;
		Rectangle	rect,
					popup_rect= new Rectangle(0, 0, 120, 100);
		TreeNodes node;
		RowLayout l;
		ArrayList<TreeNodes> nodes;
		ArrayList<Composite> comps;
		
		if(	move
			&&
			m_sMenu.equals("")	)
		{
			return;
		}
		if(!m_sMenu.equals(menu_entry))
		{
			if(m_popupShell != null)
				m_popupShell.dispose();
			popup= m_mRootEntrys.get(menu_entry);
			rect= popup.getBounds();
			m_popupShell= new Shell(m_oTopLevelShell, SWT.NO_TRIM);
			//popup_rect= m_popupShell.getClientArea();
			abs= LayoutLoader.getAbsoluteUseFieldPoint();
			popup_rect.x= m_oMenu.getBounds().x + abs.x + rect.x;
			popup_rect.y= abs.y + rect.y + rect.height;
			l= new RowLayout();
			l.type= SWT.VERTICAL;
			l.pack= false;
			l.marginBottom= 0;
			l.marginHeight= 0;
			l.marginLeft= 0;
			l.marginRight= 0;
			l.marginTop= 0;
			l.marginWidth= 0;
			l.spacing= 0;
			m_popupShell.setLayout(l);
			node= m_mRootNodes.get(menu_entry);
			nodes= node.getChilds();
			m_sMenu= menu_entry;
			if(nodes.size() == 0)
			{
				m_sMenu= "";
				return;
			}
			comps= new ArrayList<Composite>();
			for (TreeNodes subnode : nodes) 
			{
				RowLayout layout= new RowLayout();
				Composite comp= new Group(m_popupShell, SWT.NONE);
				Label text= new Label(comp, SWT.NONE);
				HashMap<String, String> metablock= subnode.getMetaData();
				String popupspace= metablock.get("popupspace");
				final String entry;
				int space= 20;
				FontObject font= new FontObject();
				int popupsize= 0;
				boolean bold= false;
				boolean italic= false;
				String looks, ssize;
				MouseListener listener1, listener2;
				
				entry= m_sMenu + "/" + subnode.getName();
				ssize= metablock.get("popupfontsize");
				if(ssize != null)
					popupsize= Integer.parseInt(ssize);
				looks= metablock.get("popupstyle");
				if(looks != null)
				{
					String[] split;
					
					split= looks.split(",");
					for(int i= 0; i < split.length; ++i)
					{
						split[i]= split[i].trim().toLowerCase();
						if(split[i].equals("bold"))
							bold= true;
						else if(split[i].equals("italic"))
							italic= true;
					}
				}
				font.defineColor(popup, metablock.get("popupcolor"), colors.WIDGET, entry + " popup");
				font.defineColor(popup, metablock.get("popupfontcolor"), colors.TEXT, entry + " popup");
				font.defineFont(popup, metablock.get("popupfont"), popupsize, bold, italic, /*underline*/false);
				text.setText(subnode.getTitle().trim());
				if(popupspace != null)
					space= Integer.parseInt(popupspace);
				layout.type= SWT.VERTICAL;
				layout.marginTop= space;
				layout.marginRight= space;
				layout.marginBottom= space;
				layout.marginLeft= space;
				comp.setLayout(layout);
				font.setDevice(comp);
				font.setDevice(text);
				font.dispose();
				comps.add(comp);
				text.addMouseListener(listener1= new MouseAdapter() 
				{
					public void mouseDown(MouseEvent event)
					{
						LayoutLoader loader;
						Thread t= null;
						
						if(HtmTags.lockDebug)
						{
							t= Thread.currentThread();
							System.out.println(t.getName()+" want to setActiveSideVisible of " + entry);
						}
						loader= LayoutLoader.instance();
						loader.m_sAktFolder= entry;
						synchronized (TreeNodes.m_DISPLAYLOCK)
						{
							loader.setActSideVisible(/*inform server by no body*/true);
						}
						m_nPopup= m_popupShell.getBounds();
						destroyPopupShell();
						m_sMenu= "";
					}
				});
				m_aPopupListeners.put(text, listener1);
				comp.addMouseListener(listener2= new MouseAdapter() 
				{
					public void mouseDown(MouseEvent event)
					{
						LayoutLoader loader;
						Thread t= null;
						
						if(HtmTags.lockDebug)
						{
							t= Thread.currentThread();
							System.out.println(t.getName()+" want to setActiveSideVisible of " + entry);
						}
						loader= LayoutLoader.instance();
						loader.m_sAktFolder= entry;
						synchronized (TreeNodes.m_DISPLAYLOCK)
						{
							loader.setActSideVisible(/*inform server by no body*/true);
						}
						m_nPopup= m_popupShell.getBounds();
						destroyPopupShell();
						m_sMenu= "";
					}
				});
				m_aPopupListeners.put(comp, listener2);
			}
			
			if(m_nPopup != null)
				popup_rect= m_nPopup;
			m_popupShell.setBounds(popup_rect);
			m_popupShell.setEnabled(false);
			m_popupShell.setVisible(false);
			m_popupShell.open();
			m_popupShell.pack();
			m_popupShell.setEnabled(true);
			m_popupShell.setVisible(true);
			
		}else if(	!m_sMenu.equals("")
					&&
					!move				)
		{// actual popup is the same
			m_popupShell.dispose();
			m_sMenu= "";
		}
	}
	/**
	 * destroy exist menu pop-up
	 * shell when open
	 */
	public void destroyPopupShell()
	{
		if(m_popupShell == null)
			return;
		// Alexander Kolli 01/02/2014
		// do not need to remove mouse listeners 
		// when I dispose the widget
/*		DisplayAdapter.syncExec(new Runnable() {
			
			public void run() 
			{
				for(Control key : m_aPopupListeners.keySet())
				{
					MouseListener listener;
					
					listener= m_aPopupListeners.get(key);
					if(listener != null)
					{
						key.removeMouseListener(listener);
						m_aPopupListeners.remove(key);
					}
				}
			}
		});*/
		m_aPopupListeners.clear();
		m_popupShell.dispose();
		m_sMenu= "";
		m_popupShell= null;
	}
}
