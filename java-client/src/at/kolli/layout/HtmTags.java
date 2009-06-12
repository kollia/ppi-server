package at.kolli.layout;

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
	public permission actPermission;
	
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
	public abstract void execute(Composite composite);

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
