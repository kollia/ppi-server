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
import java.util.HashMap;
import java.util.LinkedList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.omg.CORBA.BooleanHolder;
import org.xml.sax.SAXException;

import at.kolli.automation.client.MsgTranslator;

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
	 * name of layout file
	 */
	public String layoutName= "UNKNOWN";
	/**
	 * row of file
	 */
	public int layoutRow= 0;
	/**
	 * name of current tag
	 */
	public final String tagName;
	/**
	 * name of tag setting in the layout file
	 */
	public String name= "";
	/**
	 * main color of all widgets
	 */
	public static Color systemColor= null;
	/**
	 * after press button, move to x position
	 */
	public static int moveMouseX= -1;
	/**
	 * after press button, move to y position 
	 */
	public static int moveMouseY= -1;
	/**
	 * move mouse to new position after milliseconds
	 */
	public static int moveMouseDelay= -1;
	/**
	 * whether should load only content of current side with progress bar
	 * and all other sides in background with lower priority.<br />
	 * this can be defined inside client.ini
	 * with UseBackgroundLoadingPriority as lower or higher<br />
	 * default do not load sides as background (option not be set),
	 * because option is experimental
	 * try also by definition of this option
	 * to change -XX Command-line Options by starting client
	 * differ by -XX:(+|-)UseThreadPriorities
	 */
	public static int useBackgroundLoadingPriority= 0;
	/**
	 * boolean value for setting an debug session.<br />
	 * this flag can be set by starting
	 * with option -d
	 */
	public static boolean debug= false;
	/**
	 * string of folder which contain all pages.<br />
	 * when this variable defined with an folder name,
	 * all subroutines inside this folder will be displayed
	 * on command line where java-client will be started
	 * when changed from this client, or this client fetch an
	 * value from subroutine, or server sending over hearing
	 * connection some changes of the subroutine.<br />
	 * This behavior have to be implemented here by sourcecode
	 * and the variable should be as default an null string ("")
	 */
	public static String debugFolder= "page";
	/**
	 * when an layout page has the meta tag 'pageset'
	 * this variable allow to send value 0 to server
	 * when client leaf the page 
	 */
	public static boolean informServerLeafPage= true;
	/**
	 * boolean value to show behavior of locking 
	 * by setting new sides.<br />
	 * this flag can be set by starting
	 * with option --lock
	 */
	public static boolean lockDebug= false;
	/**
	 * boolean value to show locking behavior
	 * by SWT disply synchronization.<br />
	 * this flag can be set by starting
	 * with option --sync
	 */
	public static boolean syncSWTExec= false;
	/**
	 * whether creating and dispose
	 * of color and font objects
	 * should be shown on shell output
	 */
	public static boolean showFontCreation= false;
	/**
	 * whether should show disposition of client
	 * inside progress bar by ending.
	 * Otherwise closing doing in background
	 * which can during some minutes
	 */
	public static boolean showDisposeClient= false;
	/**
	 * boolean value to see dispatch 
	 * synchronization of shell.<br />
	 * this flag can be set by starting
	 * with option --dispatch
	 */
	public static boolean dispatchShell= false;
	/**
	 * show table structure in defined color.<br />
	 * Some operating systems, like KDE4, do not show shadows when
	 * an table defined with border="1". For this case,
	 * this should be an workaround for developing of layout
	 */
	public static String tablestructure= "";
	/**
	 * flag to use native browser inside SWT browser widget
	 * <table>
	 * 	<tr>
	 * 		<td>
	 * 			SWT.NONE
	 * 		</td>
	 * 		<td>
	 * 			-
	 * 		</td>
	 * 		<td>
	 * 			using standard browser on OS (default)
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			SWT.MOZIllA
	 * 		</td>
	 * 		<td>
	 * 			-
	 * 		</td>
	 * 		<td>
	 * 			using mozilla browser
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			SWT.WEBKIT
	 * 		</td>
	 * 		<td>
	 * 			-
	 * 		</td>
	 * 		<td>
	 * 			using webkit browser
	 * 		</td>
	 * 	</tr>
	 * </table> 
	 */
	public static int m_nUseBrowser= SWT.NONE;
	/**
	 * padding border around body of layout
	 */
	public static int mainPadding= 10;
	/**
	 * padding border around popup main buttons
	 */
	public static int popupPadding= 3;
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
	 * representing the horizontal align.<br />
	 * constant value from GridData BEGINNING, CENTER or ENDING
	 */
	public int align= -1;
	/**
	 * representing the vertical align.<br />
	 * constant value from GridData BEGINNING, CENTER or ENDING
	 */
	public int valign= -1;
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
	 * @throws SAXExecption for wrong tag handling
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected abstract void insert(HtmTags newTag) throws SAXException;
	
	/**
	 * describe whether Htm object has inside text fields
	 * 
	 * @param before text field before in the same row
	 * @param lastWasBreake whether last tag was an break tag
	 * @return count of text fields
	 * @author Alexander Kolli
	 * @version 0.02.00, 09.03.2012
	 * @since JDK 1.6
	 */
	public LinkedList<Integer> textFields(Integer before,BooleanHolder lastWasBreake)
	{	
		LinkedList<Integer> lRv= new LinkedList<Integer>();
		
		lRv.addLast(before);
		return lRv;
	}
	
	/**
	 * abstract method of execute which must have all tags
	 * to create the component to display
	 * 
	 * @param composite parent composite
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public abstract void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException;
	
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
