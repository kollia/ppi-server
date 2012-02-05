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

import java.util.HashMap;

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
	 * whether label should be an separator 
	 */
	public int separator= SWT.NONE;
	/**
	 * with of separator
	 */
	public int width= -1;
	
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
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, HashMap<String, HtmTags> classes)
	{
		GridData data= new GridData();
		org.eclipse.swt.widgets.Label label= new org.eclipse.swt.widgets.Label(composite, separator);
		
		label.setText(m_sText);		
		if(separator != SWT.NONE)
		{
			if(width == -1)
			{
				data.grabExcessHorizontalSpace= true;
				data.horizontalAlignment= GridData.FILL;
			}else
				data.widthHint= width;				
		}else
			data.horizontalAlignment= GridData.BEGINNING;
		label.setLayoutData(data);
	}

}
