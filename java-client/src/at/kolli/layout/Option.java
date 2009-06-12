package at.kolli.layout;

import org.eclipse.swt.widgets.Composite;

/**
 * representing an option tag inside of an component tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 09.12.2007
 * @since JDK 1.6
 */
public class Option extends HtmTags
{
	/**
	 * current value of the tag
	 */
	public Double value= null;
	
	/**
	 * create an instance of the option tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public Option()
	{
		super("option");
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
	 * overridden insert for inherit tags
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of tr tag
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void insert(HtmTags newTag)
	{
		if(newTag instanceof Label)
			m_lContent.add(newTag);
	}

	/**
	 * method returning the string inherit of the option tag
	 * 
	 * @return string inherit of the option tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public String getOptionString()
	{
		Label label;
		
		if(m_lContent.isEmpty())
			return "";
		label= (Label)m_lContent.get(0);
		return label.getText();
	}
}
