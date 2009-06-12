package at.kolli.layout;

import org.eclipse.swt.widgets.Composite;

/**
 * class representing the head-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Head extends HtmTags
{
	/**
	 * creating instance of head-tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Head() 
	{
		super("head");
	}

	/**
	 * Dummy method extended from {@link HtmTags} which should generate some components
	 * into the displaying window. But this class have nothing to display, so method has nothing to do.
	 * 
	 * @param composite parent {@link Composite}
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite)
	{
		// this tag do not display anything
	}

	/**
	 * overridden insert for inherit tags.<br />
	 * only for tags which are allowed in the head
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of head tag
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void insert(HtmTags newTag)
	{
		m_lContent.add(newTag);
		newTag.m_oParent= this;
	}
	
	/**
	 * returning the real name of the subroutine,
	 * which should displayed on the left side
	 * 
	 * @return
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Title getTitle()
	{
		for(HtmTags tag : m_lContent)
		{
			if(tag instanceof Title)
			{
				Title title= (Title)tag;
				return title;
			}
		}
		return null;
	}
}
