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
import java.util.LinkedList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.omg.CORBA.BooleanHolder;
import org.xml.sax.SAXException;

import at.kolli.layout.FontObject.colors;

/**
 * class representing the head-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Style extends HtmTags
{

	/**
	 * current name of font
	 */
	public String font= "";
	/**
	 * character size from system font
	 */
	public int size= 0;
	/**
	 * whether font should be bold
	 */
	public boolean bold= false;
	/**
	 * whether font should be italic
	 */
	public boolean italic= false;
	/**
	 * whether font should be underlined
	 */
	public boolean underline= false;
	/**
	 * current color
	 */
	public String color= "";
	/**
	 * which type of color should be defined
	 */
	public colors colortype= colors.TEXT;
	/**
	 * whether tag is defined inside
	 * an legend tag
	 */
	public boolean legend= false;
	
	/**
	 * creating instance of head-tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public Style() 
	{
		super("font");
	}

	/**
	 * method to create font for next composites/widgets-<br />
	 * This method will be never called and exists only while method is abstract inside HTMTag object
	 * 
	 * @param composite parent {@link Composite}
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 10.03.2012
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException
	{
		// This method will be never called and exists only while method is abstract inside HTMTag object
		// the called objects from ContentField and Style calling only the other execute() method
	}
	/**
	 * define new font and color object
	 * 
	 * @param composite device to get display for font creation
	 * @param oldObj current FontObject
	 * @param bCreated when this parameter is set true, method create an new object when needed and give back also true, otherwise false
	 * @return new object when defined, otherwise old
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 10.03.2012
	 * @since JDK 1.6
	 */
	public FontObject defineObject(Composite composite, FontObject oldObj, BooleanHolder bCreated)
	{
		boolean isCreated= false;
		boolean shouldCreate= bCreated.value;
		FontObject newFont;
		
		newFont= oldObj.defineNewFontObj(composite, this.font, size, bold, italic, underline, bCreated);
		if(bCreated.value)
		{
			isCreated= true;		// when inside defineNewFontObj an new object was created
			bCreated.value= false;	// create no new one by defineNewColorObj
			
		}else if(shouldCreate)
			bCreated.value= true;	
		newFont= newFont.defineNewColorObj(composite, color, colortype, bCreated, layoutName);
		if(	bCreated.value == false &&
			isCreated == true			)
		{
			bCreated.value= true;
		}
		return newFont;
	}
	/**
	 * method to create font for next composites/widgets
	 * 
	 * @param composite parent {@link Composite}
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 10.03.2012
	 * @since JDK 1.6
	 */
	public Composite execute(Composite rowCp, Composite field, FontObject font, HashMap<String, HtmTags> classes, LinkedList<Integer> columns) throws IOException
	{
		GridLayout fieldLayout;
		GridData fieldData;
		BooleanHolder newObj= new BooleanHolder();
		FontObject newFont= null;
		
		newObj.value= true;
		newFont= defineObject(rowCp, font, newObj);
		for (HtmTags tag : m_lContent)
		{
			tag.align= align;
			tag.valign= valign;
			if(tag instanceof Break)
			{
				field= new Composite(rowCp, SWT.NONE);
				fieldLayout= new GridLayout();
				fieldData= new GridData();
				
				newFont.setDevice(field);
				fieldData.horizontalAlignment= align;
				fieldData.grabExcessHorizontalSpace= true;
				fieldLayout.marginHeight= 0;
				fieldLayout.marginWidth= 0;
				fieldLayout.marginLeft= 0;
				fieldLayout.marginRight= 0;
				fieldLayout.marginTop= 0;
				fieldLayout.marginBottom= 0;
				fieldLayout.horizontalSpacing= 0;
				fieldLayout.verticalSpacing= 0;
				fieldLayout.numColumns= columns.getFirst();
				columns.removeFirst(); //<- for next list index
				field.setLayout(fieldLayout);
				field.setLayoutData(fieldData);
				
			}else if(tag instanceof Style)
				field= ((Style)tag).execute(rowCp, field, newFont, classes, columns);
			else
				tag.execute(field, newFont, classes);
		}
		if(newObj.value)
			newFont.dispose();
		return field;
	}
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
	public LinkedList<Integer> textFields(Integer before, BooleanHolder lastWasBreake)
	{
		int o= 0;
		HtmTags tag;
		Integer val= before;
		LinkedList<Integer> lRv= new LinkedList<Integer>();
		LinkedList<Integer> rRv;

		while(o < m_lContent.size())
		{
			tag= m_lContent.get(o);
			if(tag instanceof Label)
			{
				//System.out.println("count: " + ((Label)tag).m_sText);
				++val;
				lastWasBreake.value= false;
				
			}else if(tag instanceof Style)
			{
				rRv= tag.textFields(val, lastWasBreake);
				for(int i= 0; i < rRv.size()-1; ++i)
					lRv.addLast(rRv.get(i));
				val= rRv.get(rRv.size()-1);
			}
			else if(tag instanceof Break)
			{
				if(lastWasBreake.value)
				{
					Label lable= new Label();
					
					lable.setText(" ");
					m_lContent.add(o, lable);
					++o;
					++val;
				}
				lRv.addLast(val);
				val= 0;
				lastWasBreake.value= true;
				
			}else
			{
				++val;
				lastWasBreake.value= false;
			}
			++o;
		}
		lRv.addLast(val);
		return lRv;
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
	public void insert(HtmTags newTag) throws SAXException
	{
		boolean bInsert= true;
		
		if(legend)
		{
			HtmTags curTag= this;
			
			bInsert= false;
			while(curTag != null)
			{
				if(curTag instanceof Legend)
				{
					if(((Legend)curTag).allowedTag(newTag))
						bInsert= true;
					break;
				}
				curTag= curTag.getParentTag();
			}
		}
		if(bInsert)
		{
			m_lContent.add(newTag);
			newTag.m_oParent= this;
		}
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
		
		obj= defineObject(composite, obj, bCreated);
		for (HtmTags tag : m_lContent)
		{
			if(bCreated.value)
			{
				bCreated.value= false;
				isCreated= true;
				shouldCreate= false; // no second object should be created
				
			}else if(shouldCreate)
				bCreated.value= true;
			if(tag instanceof Style)
				obj= ((Style)tag).getFontObject(composite, obj, bCreated);
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
