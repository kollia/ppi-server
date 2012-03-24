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
import java.util.HashMap;

import org.eclipse.swt.widgets.Composite;
import org.omg.CORBA.BooleanHolder;
import org.xml.sax.SAXException;

/**
 * class representing the body-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 0.02.00, 20.03.2012
 * @since JDK 1.6
 */
public class Legend extends HtmTags
{	
	/**
	 * whether currently be inserted normally text
	 */
	private boolean m_bText= false;
	
	/**
	 * creating instance of body tag
	 * 
	 * @since JDK 1.6
	 */
	public Legend()
	{
		super("legend");
		//nextLine();
		//insert(new ContentFields());
	}
	/**
	 * Overridden method of execute
	 * this method is never used
	 * 
	 * @param composite parent {@link Composite}
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 20.3.2012
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException
	{
	}
	/**
	 * overridden insert for inherit tags
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of legend tag
	 * @throws SAXExecption for wrong tag handling
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 20.3.2012
	 * @since JDK 1.6
	 */
	public void insert(HtmTags newTag) throws SAXException
	{
		if(allowedTag(newTag))
		{
			newTag.m_oParent= this;
			m_lContent.add(newTag);
		}
	}
	/**
	 * whether new inserted tag will be allowed to insert
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of legend tag
	 * @return whether allowed
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 20.3.2012
	 * @since JDK 1.6
	 */
	public boolean allowedTag(HtmTags newTag) throws SAXException
	{
		if(newTag instanceof Style)
		{
			if(m_bText)
				throw new SAXException("inside an legend tag can not be defined differend text fonts");
			((Style)newTag).legend= true;
			
		}else if(newTag instanceof Label)
		{
			m_bText= true;
		}else
			throw new SAXException("inside tag <legend> can only be defined text or some style-tags (<font>, <b>, <i>)");
		return true;
	}
	/**
	 * get new font object for legend
	 * 
	 * @param composite device to get display for font creation
	 * @param obj old font object
	 * @param bCreated give back true when new font object created
	 * @author Alexander Kolli
	 * @version 0.02.00, 20.3.2012
	 * @since JDK 1.6
	 */
	public FontObject getFontObject(Composite composite, FontObject obj, BooleanHolder bCreated)
	{
		boolean isCreated= false;
		boolean shouldCreate= bCreated.value;
		
		for (HtmTags tag : m_lContent)
		{
			if(tag instanceof Style)
				obj= ((Style)tag).getFontObject(composite, obj, bCreated);
			if(bCreated.value)
			{
				bCreated.value= false;
				isCreated= true;
				shouldCreate= false; // no second object should be created
				
			}else if(shouldCreate)
				bCreated.value= true;
		}
		if(	bCreated.value == false &&
				isCreated == true			)
		{
			bCreated.value= true;
		}
		return obj;
	}
	/**
	 * return text from Label objects
	 * 
	 * @return inherited text
	 * @author Alexander Kolli
	 * @version 0.02.00, 20.3.2012
	 * @since JDK 1.6
	 */
	public String getText()
	{
		String sRv= "";
		
		for (HtmTags tag : m_lContent)
		{
			if(tag instanceof Style)
				sRv+= ((Style)tag).getText();
			else if(tag instanceof Label)
				sRv+= ((Label)tag).toString();
		}
		return sRv;
	}
	
}
