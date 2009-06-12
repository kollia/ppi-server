package at.kolli.layout;

import java.util.ArrayList;

import org.eclipse.swt.widgets.Composite;

/**
 * class representing first all include tag &lt;layout&gt;
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Layout extends HtmTags
{
	/**
	 * creating instance of layout-tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Layout()
	{
		super("layout");
	}

	/**
	 * Dummy method extended from {@link HtmTags} which should generate some components
	 * into the displaying window. But this class have nothing to display, so methode has nothing to do.
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
	 * only for tag head and body
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of layout tag
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
	 * returning the registered {@link Head} tag from layout file
	 * 
	 * @return {@link Head} tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Head getHead()
	{
		for(HtmTags tag : m_lContent)
		{
			if(tag instanceof Head)
			{
				Head head= (Head)tag;
				return head;
			}
		}
		return null;
	}
	
	/**
	 * returning the registered {@link Body} tags from layout file
	 * in an {@link ArrayList}
	 * 
	 * @return ArrayList of body-tags
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public ArrayList<Body> getBody()
	{
		ArrayList<Body> bodyTags= new ArrayList<Body>();
		
		for(HtmTags tag : m_lContent)
		{
			if(tag instanceof Body)
			{
				Body body= (Body)tag;
				bodyTags.add(body);
			}
		}
		return bodyTags;
	}
}
