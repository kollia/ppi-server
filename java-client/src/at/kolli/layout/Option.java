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
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes)
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
