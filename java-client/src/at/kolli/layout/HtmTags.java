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

import java.io.IOException;
import java.util.ArrayList;

import org.eclipse.swt.widgets.Composite;

/**
 * Abstract base-class of all tags
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 04.12.2007
 * @since JDK 1.6
 */
public abstract class HtmTags 
{
	/**
	 * name of current tag
	 */
	public final String tagName;
	/**
	 * name of tag setting in the layout file
	 */
	public String name= "";
	/**
	 * boolean value for setting an debug session
	 */
	public static boolean debug= false;
	/**
	 * show also sides defined with meta tag display false
	 */
	public static boolean showFalse= false;
	/**
	 * whether set window to full screen
	 */
	public static boolean fullscreen= false;
	/**
	 * whether display normally menu (refresh/change user/exit) in menu-bar
	 */
	public static boolean nomenu= false;
	/**
	 * whether display sides on left side in an tree by false,<br>
	 * otherwise display the sides in the menu-bar
	 */
	public static boolean notree= false;
	/**
	 * whether the main window have an title-bar
	 */
	public static boolean notitle= false;
	/**
	 * all rows of component-list which can also inherit one ore more components
	 */
	protected ArrayList<ArrayList<HtmTags>> m_lRows= new ArrayList<ArrayList<HtmTags>>();
	/**
	 * how much tags in row exists
	 */
	protected int m_nContent= -1;
	/**
	 * current row inherits tags
	 */
	protected ArrayList<HtmTags> m_lContent;
	/**
	 * composite or group for the tag
	 */
	protected Composite m_oComposite;
	/**
	 * parent tag
	 */
	protected HtmTags m_oParent= null;
	/**
	 * actual permission of component
	 */
	private permission actPermission= permission.writeable;
	
	/**
	 * getter routine to get actual permission from outside
	 * 
	 * @return actual permission
	 */
	public permission getPermission()
	{
		return actPermission;
	}
	
	/**
	 * setter routine to set actual permission inside of an tag
	 * 
	 * @param perm permission to set
	 */
	protected void setPermission(permission perm)
	{
		actPermission= perm;
	}
	/**
	 * base constructor of all tags
	 * 
	 * @param name name of tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public HtmTags(String name)
	{
		ArrayList<HtmTags> list= new ArrayList<HtmTags>();
		
		this.tagName= name;
		m_lContent= list;
		m_lRows.add(list);
	}
	
	/**
	 * abstract method of insert which must have all tags
	 * 
	 * @param newTag instance of an new tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected abstract void insert(HtmTags newTag);
	
	/**
	 * abstract method of execute which must have all tags
	 * to create the component to display
	 * 
	 * @param composite parent composite
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public abstract void execute(Composite composite) throws IOException;

	/**
	 * returning the parent tag
	 * 
	 * @return parent tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public HtmTags getParentTag()
	{
		return m_oParent;
	}
}
