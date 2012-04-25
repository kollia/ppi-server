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

import at.kolli.automation.client.MsgClientConnector;

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
	private boolean m_bBorder= false;
	
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
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void insert(HtmTags newTag)
	{
		HtmTags list;
		
		if(newTag instanceof ContentFields)
		{
			ContentFields field= (ContentFields)newTag;
			
			field.setBorder(m_bBorder);
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
	 * calculate the highest height per column
	 * 
	 * @return the highest height 
	 * @author Alexander Kolli
	 * @version 1.00.00, 24.09.2011
	 * @since JDK 1.6
	 */
	public int getHighestField()
	{
		int highest= -1;
		
		for(HtmTags tag : m_lContent)
		{
			ContentFields field= (ContentFields)tag;
			
			if(field.height > highest)
				highest= field.height;
		}
		return highest;
	}
	
	/**
	 * set the highest height in each column
	 * 
	 * @param height highest height
	 * @author Alexander Kolli
	 * @version 1.00.00, 24.09.2011
	 * @since JDK 1.6
	 */
	public void setHighestField(int height)
	{
		for(HtmTags tag : m_lContent)			
		{
			ContentFields field= (ContentFields)tag;
			
			field.height= height;
		}
	}
	
	/**
	 * calculate the maximal width for each column
	 * 
	 * @return maximal width for each column
	 * @author Alexander Kolli
	 * @version 1.00.00, 24.09.2011
	 * @since JDK 1.6
	 */
	public ArrayList<Integer> getMaxWidth()
	{
		ArrayList<Integer> aRv= new ArrayList<Integer>();

		for(HtmTags tag : m_lContent)			
		{
			ContentFields field= (ContentFields)tag;
			
			aRv.add(field.width);
		}
		return aRv;
	}
	
	/**
	 * set the maximal width in each column
	 * 
	 * @param width maximal width
	 * @author Alexander Kolli
	 * @version 1.00.00, 24.09.2011
	 * @since JDK 1.6
	 */
	public void setMaxWidth(ArrayList<Integer> width)
	{
		int count= 0;

		for(HtmTags tag : m_lContent)			
		{
			ContentFields field= (ContentFields)tag;
			
			field.width= width.get(count);
			++count;
		}
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
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, HashMap<String, HtmTags> classes) throws IOException
	{
		execute(composite, classes, null);
	}
	
	/**
	 * method to generate the widget in the display window
	 * 
	 * @param composite parent {@link Composite}
	 * @param classes all class definition for any tags
	 * @param isAlsoNextColumn ArrayList to see how much rows the field in the row before needed
	 * @author Alexander Kolli
	 * @version 1.00.00, 07.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, HashMap<String, HtmTags> classes, ArrayList<Integer> isAlsoNextColumn) throws IOException
	{
		int column= 0;
		int maxColumns= isAlsoNextColumn.size();
		
		askPermission();
		if(getPermission().compareTo(permission.readable) == -1)
			return;
		for(HtmTags tag : m_lContent)
		{
			boolean again= false;
			Integer fromTop;
			
			do{
				fromTop= isAlsoNextColumn.get(column);
				if(fromTop > 0)
				{
					--fromTop;
					isAlsoNextColumn.set(column, fromTop);
					++column;
					again= true;
				}else
					again= false;
				
			}while(again);
			
			isAlsoNextColumn.set(column, fromTop + ((ContentFields)tag).rowspan -1);
			column+= ((ContentFields)tag).colspan;
			tag.execute(composite, classes);
		}
		while(column < maxColumns)
		{
			ContentFields field= new ContentFields();
			boolean again= false;
			Integer fromTop;
			Label label;
			
			do{
				fromTop= isAlsoNextColumn.get(column);
				if(fromTop > 0)
				{
					--fromTop;
					isAlsoNextColumn.set(column, fromTop);
					++column;
					again= true;
				}else
					again= false;
				
			}while(again && column < maxColumns);
			
			if(column < maxColumns)
			{
				insert(field);
				field.setBorder(m_bBorder);
				label= new Label();
				label.setText(" ");
				field.insert(label);
				field.execute(composite, classes);
				++column;
			}
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
	public void setBorder(boolean set)
	{
		m_bBorder= set;
	}
}
