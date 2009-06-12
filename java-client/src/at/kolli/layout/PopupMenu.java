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
import org.eclipse.swt.events.MouseMoveListener;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Shell;

import at.kolli.automation.client.LayoutLoader;
import at.kolli.automation.client.TreeNodes;

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
	 * root nodes from all readed trees
	 */
	private Hashtable<String, TreeNodes> m_mRootNodes= new Hashtable<String, TreeNodes>();
	/**
	 * all main entrys in the menu with defined group composite
	 */
	private Hashtable<String, Group> m_mRootEntrys= new Hashtable<String, Group>();
	/**
	 * inherits the name of menu which apear currently on screen.
	 * Otherwise the string is ""
	 */
	private String m_sMenu= "";
	
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
	 * creating new object of popup-menu if not exist
	 * and set main menu entry
	 * 
	 * @param menubar composite where the root of menues should appear
	 * @param string of one entry on the root
	 * @param metablock all meta data defined inside of the head tag
	 */
	public static void init(Composite menubar, TreeNodes nodes)
	{
		RowLayout layout= new RowLayout();
		final Group popup= new Group(menubar, SWT.NONE);
		Label text= new Label(popup, SWT.SHADOW_IN);
		HashMap<String, String> metablock= nodes.getMetaData();
		final String entry;
		String spacing= metablock.get("popupspace");
		int space;
		
		entry= nodes.getName();
		if(_instance == null)
		{
			_instance= new PopupMenu(menubar);
		}

		text.setText(entry);
		if(spacing != null)
			space= Integer.parseInt(spacing);
		else
			space= 15;
		layout.marginTop= space;
		layout.marginRight= space + (space /2);
		layout.marginBottom= space;
		layout.marginLeft= space + (space /2);
		popup.setLayout(layout);
		text.addMouseListener(new MouseAdapter() 
		{
			public void mouseDown(MouseEvent event)
			{
				LayoutLoader loader;
				
				_instance.show(entry, false);
				synchronized (TreeNodes.m_DISPLAYLOCK)
				{
					loader= LayoutLoader.instance();
					loader.m_sAktFolder= entry;
					loader.setActSideVisible();
				}
			}
		});
		popup.addMouseListener(new MouseAdapter() 
		{
			public void mouseDown(MouseEvent event)
			{
				LayoutLoader loader;
				
				_instance.show(entry, false);
				synchronized (TreeNodes.m_DISPLAYLOCK)
				{
					loader= LayoutLoader.instance();
					loader.m_sAktFolder= entry;
					loader.setActSideVisible();
				}
			}
		});
		text.addMouseMoveListener(new MouseMoveListener()
		{
			public void  mouseMove(MouseEvent ev)
			{
				_instance.show(entry, true);
			}
		});
		popup.addMouseMoveListener(new MouseMoveListener()
		{
			public void  mouseMove(MouseEvent ev)
			{
				_instance.show(entry, true);
			}
		});
		_instance.m_mRootEntrys.put(entry, popup);
		_instance.m_mRootNodes.put(entry, nodes);
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
					popup_rect;
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
			popup_rect= m_popupShell.getClientArea();
			abs= LayoutLoader.getAbsoluteUseFieldPoint();
			popup_rect.x= m_oMenu.getBounds().x + abs.x + rect.x;
			popup_rect.y= abs.y + rect.y + rect.height;
			l= new RowLayout();
			l.type= SWT.VERTICAL;
			l.pack= false;
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
				
				entry= m_sMenu + ":" + subnode.getName();
				text.setText(subnode.getName());
				if(popupspace != null)
					space= Integer.parseInt(popupspace);
				layout.type= SWT.VERTICAL;
				layout.marginTop= space;
				layout.marginRight= space;
				layout.marginBottom= space;
				layout.marginLeft= space;
				comp.setLayout(layout);
				comps.add(comp);
				text.addMouseListener(new MouseAdapter() 
				{
					public void mouseDown(MouseEvent event)
					{
						LayoutLoader loader;
						
						//_instance.show(entry, false);
						loader= LayoutLoader.instance();
						loader.m_sAktFolder= entry;
						synchronized (TreeNodes.m_DISPLAYLOCK)
						{
							loader.setActSideVisible();
						}
						destroy();
						m_sMenu= "";
					}
				});
				comp.addMouseListener(new MouseAdapter() 
				{
					public void mouseDown(MouseEvent event)
					{
						LayoutLoader loader;
						
						//_instance.show(entry, false);
						loader= LayoutLoader.instance();
						loader.m_sAktFolder= entry;
						synchronized (TreeNodes.m_DISPLAYLOCK)
						{
							loader.setActSideVisible();
						}
						destroy();
						m_sMenu= "";
					}
				});
				/*text.addMouseMoveListener(new MouseMoveListener()
				{
					public void  mouseMove(MouseEvent ev)
					{
						//_instance.show(entry, true);
					}
				});
				comp.addMouseMoveListener(new MouseMoveListener()
				{
					public void  mouseMove(MouseEvent ev)
					{
						//_instance.show(entry, true);
					}
				});*/
			}

			int height= 0;
			int width= 0;
			
			m_popupShell.setBounds(popup_rect);
			//m_popupShell.setEnabled(false);
			m_popupShell.setVisible(false);
			m_popupShell.open();
			popup_rect.height= 0;
			popup_rect.width= 0;
			for (Composite composite : comps) 
			{
				rect= composite.getBounds();
				if(	height == 0
					&&
					width == 0	)
				{
					height= rect.y * 2;
					width= rect.x;
				}
				popup_rect.height+= rect.height + height;
				if(rect.width > popup_rect.width)
					popup_rect.width= rect.width + rect.x + 4;
			}
			m_popupShell.setBounds(popup_rect);
			//m_popupShell.setEnabled(true);
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
	 * destroy all menu popups
	 */
	public void destroy()
	{
		if(m_popupShell == null)
			return;
		m_popupShell.dispose();
		m_sMenu= "";
		m_popupShell= null;
	}
}
