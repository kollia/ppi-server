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
import java.security.acl.Group;
import java.util.ArrayList;
import java.util.HashMap;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.xml.sax.SAXException;

import at.kolli.automation.client.MsgClientConnector;
import at.kolli.automation.client.MsgTranslator;

/**
 * class representing an row in an table for layout on display
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 04.12.2007
 * @since JDK 1.6
 */
public class ContentRows extends HtmTags
{	
	/**
	 * representing the offset from inner field to content
	 */
	public int cellpadding;
	/**
	 * representing the horizontal align.<br />
	 * constant value from GridData BEGINNING, CENTER or ENDING
	 */
	public int align;
	/**
	 * representing the vertical align.<br />
	 * constant value from GridData BEGINNING, CENTER or ENDING
	 */
	public int valign;
	/**
	 * with of the hole row
	 */
	public int width= -1;
	/**
	 * height of all columns in the row
	 */
	public int height= -1;
	/**
	 * group name for permission
	 */
	public String permgroup= "";
	/**
	 * whether the user want to see an border by all composites<br />
	 * in this case it will be create an  {@link Group} control
	 */
	private int m_nBorder= 0;
	/**
	 * color of table field or body
	 */
	public String bgcolor= "";
	
	/**
	 * create instance of tr-tag
	 * 
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public ContentRows()
	{
		super("tr");
	}
	
	/**
	 * overridden insert for inherit tags
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of tr tag
	 * @throws SAXExecption for wrong tag handling
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void insert(HtmTags newTag) throws SAXException
	{
		HtmTags list;
		
		if(newTag instanceof ContentFields)
		{
			ContentFields field= (ContentFields)newTag;
			
			field.setBorder(m_nBorder);
			field.cellpadding= this.cellpadding;
			++m_nContent;
			m_lContent.add(newTag);
			return;
		}
		list= m_lContent.get(m_nContent);
		list.insert(newTag);
	}
	
	/**
	 * returning how much td tags ({@link ContentFields}) be in this row tag
	 * 
	 * @return count of {@link ContentFields}
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public int getMaxColumns(ArrayList<Integer> rowSpanColumns)
	{
		int columns= 0;
		int aktRowSpan;
		
		for(HtmTags tag : m_lContent)
		{
			ContentFields field= (ContentFields)tag;

			while(rowSpanColumns.size() >= columns+1)
			{
				aktRowSpan= rowSpanColumns.get(columns);
				if(aktRowSpan > 0)
					++columns;
				else
					break;
			}
			for(int i= 0; i < field.colspan; ++i)
			{
				if(columns+1 > rowSpanColumns.size())	
					rowSpanColumns.add(field.rowspan);
				else
					rowSpanColumns.set(columns, field.rowspan);
				++columns;
			}
		}
		return columns;
	}
	/**
	 * fill row with lost columns
	 * 
	 * @param columns maximal columns
	 * @param rownr actual row number
	 * @author Alexander Kolli
	 * @version 0.02.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void fillMaxColumns(int rownr, ArrayList<Integer> rowspanColumns)
	{
		boolean again= false;
		Integer fromTop;
		int column= 0, set;
		int maxColumns= rowspanColumns.size();

		for(HtmTags tag : m_lContent)
		{
			
			do{
				fromTop= rowspanColumns.get(column);
				if(fromTop > 0)
				{
					--fromTop;
					rowspanColumns.set(column, fromTop);
					++column;
					// go to next column,
					// because this should be the right one
					again= true;
				}else
					again= false;
				
			}while(again);
			
			set= fromTop + ((ContentFields)tag).rowspan -1;
			for(int i= 0; i < ((ContentFields)tag).colspan; ++i)
			{
				fromTop= rowspanColumns.get(column);
				if(fromTop > 0)
				{
					MsgTranslator.instance().errorPool("WRONG_colspan_number", layoutName, Integer.toString(rownr));
					((ContentFields)tag).colspan= i;
					break;
				}
				rowspanColumns.set(column, set);
				++column;
			}
		}
		again= false;
		// fill table with forgotten columns
		while(column < maxColumns)
		{
			fromTop= rowspanColumns.get(column);
			if(fromTop > 0)
			{
				--fromTop;
				rowspanColumns.set(column, fromTop);
			}else
			{
				ContentFields field= new ContentFields();
				
				field.layoutName= layoutName;
				m_lContent.add(field);
				again= true;
			}
			++column;
		}
		if(again)
			MsgTranslator.instance().errorPool("WARNING_add_columns", layoutName, Integer.toString(rownr));
	}
	
	/**
	 * check permission on server for this component
	 * 
	 * @throws IOException
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void askPermission() throws IOException
	{
    	MsgClientConnector client= MsgClientConnector.instance();
    	permission perm;
    	
    	if(!permgroup.equals(""))
    	{
	    	perm= client.permission(permgroup, /*bthrow*/false);
	    	if(perm == null)
	    	{
	    		setPermission(permission.None);
	    		
			}else if(perm.compareTo(getPermission()) < -1)
	    		setPermission(perm);
    	}
	}
	/**
	 * Overridden method of execute
	 * which start the execute in fields class (td) to calibrate
	 * on screen (in window)
	 * 
	 * @param composite parent {@link Composite}
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 0.02.00, 06.3.2012
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException
	{
		askPermission();
		if(getPermission().compareTo(permission.readable) == -1)
			return;
		for(HtmTags tag : m_lContent)
		{
			if(	!bgcolor.equals("") &&
				((ContentFields)tag).bgcolor.equals("")	)
			{
				((ContentFields)tag).bgcolor= bgcolor;
			}
			tag.execute(composite, font, classes);
		}
	}

	/**
	 * by setting true it will be create an border of all fields inside the table.<br />
	 * The execute method set by true an {@link Group}-control, by false
	 * an {@link Composite}
	 * 
	 * @param set true for set Border, otherwise false
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void setBorder(int set)
	{
		m_nBorder= set;
	}
}
