package at.kolli.layout;

import org.eclipse.swt.widgets.Composite;

/**
 * class representing the br-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Break extends HtmTags
{
	/**
	 * creating instance of br-tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Break()
	{
		super("br");
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
		// nothing to do in an br tag

	}

	/**
	 * Dummy method extended from {@link HtmTags} which should fill other components
	 * into this class. But his tag cannot insert any other tags, so method has nothing to do.
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of head tag
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	protected void insert(HtmTags newTag)
	{
		// nothing to do in an br tag

	}

}
