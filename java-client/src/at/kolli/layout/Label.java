package at.kolli.layout;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;

/**
 * class representing all text inside of the body-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Label extends HtmTags 
{
	/**
	 * the text which should displayed
	 */
	private String m_sText= "";
	
	/**
	 * creating instance of class to inherit text
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Label()
	{
		super("label");
	}

	/**
	 * Dummy method extended from {@link HtmTags} which should insert
	 * some other tags. But this class have nothing to display, so method has nothing to do.
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of head tag
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	protected void insert(HtmTags newTag)
	{
		// this tag is only an sinle Tag

	}
	
	/**
	 * method to insert an text into the class
	 * 
	 * @param text text to display
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void setText(String text)
	{
		m_sText= text;
	}
	
	/**
	 * returning inherited text
	 * 
	 * @return text which be inherited
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public String getText()
	{
		return m_sText;
	}

	/**
	 * Overridden method of execute
	 * which calibrate the text to display
	 * on screen (in window)
	 * 
	 * @param composite parent {@link Composite}
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite)
	{
		GridData data= new GridData();
		org.eclipse.swt.widgets.Label label= new org.eclipse.swt.widgets.Label(composite, SWT.SHADOW_IN);
		
		label.setText(m_sText);
		data.horizontalAlignment= GridData.BEGINNING;
		label.setLayoutData(data);
	}

}
