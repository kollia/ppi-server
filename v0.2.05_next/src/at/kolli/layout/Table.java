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
import java.util.ArrayList;
import java.util.HashMap;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.omg.CORBA.BooleanHolder;
import org.xml.sax.SAXException;

import at.kolli.automation.client.MsgClientConnector;
import at.kolli.layout.FontObject.colors;

//import at.kolli.layout.Component.permission;

/**
 * class representing an table for layout on display
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 04.12.2007
 * @since JDK 1.6
 */
public class Table extends HtmTags 
{
	/**
	 * representing the offset from field to table-border
	 */
	public int cellspacing= 3;
	/**
	 * representing the offset from inner field to content
	 */
	public int cellpadding= 3;
	/**
	 * representing the horizontal align.<br />
	 * constant value from GridData BEGINNING, CENTER or ENDING
	 */
	public int align= GridData.BEGINNING;
	/**
	 * representing the vertical align.<br />
	 * constant value from GridData BEGINNING, CENTER or ENDING
	 */
	public int valign= GridData.CENTER;
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
	 * color of table
	 */
	public String bgcolor= "";
	/**
	 * actual field (column) in the row
	 */
	protected ContentFields m_oAktField= null;
	/**
	 * whether filling inside an row-tag (tr)
	 */
	protected ContentRows m_oAktRow= null;
	/**
	 * whether the user want to see an border by all composites<br />
	 * in this case it will be created groups
	 */
	private int m_nBorder= 0;	
	
	/**
	 * constructor of table-tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public Table()
	{
		super("table");
	}
	
	/**
	 * constructor to overload
	 * 
	 * @param name new name of overloaded tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected Table(String name)
	{
		super(name);
	}

	/**
	 * insert method to write new content into table
	 * 
	 * @param newTag new opject of tag
	 * @throws SAXExecption for wrong tag handling
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected void insert(HtmTags newTag) throws SAXException
	{
		HtmTags list;
		ContentRows row;
		
		if(	!(newTag instanceof ContentFields) ||
			newTag instanceof FieldSet				)
		{
			if(m_oAktField == null)
			{
				String[] split;
				String firstTag, secondTag, curTag;
				
				if(m_oAktRow != null)
				{
					firstTag= "<tr>";
					secondTag= "<td>";
					
				}else
				{
					firstTag= "<table>";
					secondTag= "<tr>";
				}
				System.out.println();
				System.out.println("no correct table handling.");
				System.out.println("tag table must have follow content:");
				System.out.println("                 <table>");
				System.out.println("                    <tr>");
				System.out.println("                       <td>");
				System.out.println("                         other tags or text");
				System.out.println("                       </td>");
				System.out.println("                    </tr>");
				System.out.println("                 </table>");
				System.out.println();
				if(newTag instanceof Label)
				{
					throw new SAXException( "no text characters be allowed between " + firstTag
							+ "- and " + secondTag +"-tag.");
					
				}else
				{
					if(newTag instanceof Style)
						curTag= "font";
					else
					{
						curTag= newTag.getClass().getName();
						split= curTag.split("\\.");
						curTag= split[split.length - 1].toLowerCase();
					}
					throw new SAXException( "no <" + curTag + ">-tag be allowed between " + firstTag
												+ "- and " + secondTag +"-tag.");
				}
			}
			newTag.m_oParent= this;
			m_oAktField.insert(newTag);
			return;
		}
		newTag.m_oParent= this;
		list= m_lContent.get(m_nContent);		
		list.insert(newTag);
		if(newTag instanceof ContentFields)
		{
			m_oAktField= (ContentFields)newTag;
			m_oAktField.setBorder(m_nBorder);
			m_oAktField.align= align;
			m_oAktField.valign= valign;
		}else
		{
			row= (ContentRows)newTag;
			row.setBorder(m_nBorder);
			row.align= align;
			row.valign= valign;
			m_oAktRow= row;
		}
	}

	/**
	 * method create an new row
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public ContentRows nextLine()
	{
		m_oAktRow= new ContentRows();
		
		++m_nContent;
		m_oAktRow.setBorder(m_nBorder);
		m_oAktRow.cellpadding= this.cellpadding;
		m_lContent.add(m_oAktRow);
		m_oAktField= null;
		return m_oAktRow;
	}
	/**
	 * XMLSaxParser can so signal end of tr- or td-tag 
	 */
	public void tagEnd(String curTag)
	{
		if(curTag.equals("tr"))
			m_oAktRow= null;
		else if(curTag.equals("td"))
			m_oAktField= null;
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
	 * execute method to create the composite for display
	 * 
	 * @param composite parent composite
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException
	{
		int columns= 1;
		int rownr= 1;
		//ArrayList<Integer> maxwidth= new ArrayList<Integer>();
		GridLayout layout= new GridLayout();
		ArrayList<Integer> rowspanColumns= new ArrayList<Integer>();
		FontObject newFont, structureFont;
		BooleanHolder bHolder= new BooleanHolder();
		
		askPermission();
		if(getPermission().equals(permission.None))
			return;
		// calculate the highest count of columns in all rows
		for(HtmTags tag : m_lContent)
		{
			ContentRows row= (ContentRows)tag;
			int c= row.getMaxColumns();
			
			if(c > columns)
				columns= c;
		}		
		// define for each columns
		// whether it gets an rowspan 
		// from one of the last row
		for(int o= 0; o<columns; ++o)
			rowspanColumns.add(0);
		// calculate the highest height per column
		// and fill also the the lost columns per row
		for(HtmTags tag : m_lContent)
		{
			ContentRows row= (ContentRows)tag;
			row.fillMaxColumns(rownr, rowspanColumns);
			++rownr;
		}
		
		if(m_nBorder == 1)
		{
			m_oComposite= new Group(composite, SWT.SHADOW_IN);
			
		}else
		{
			m_oComposite= new Composite(composite, SWT.NONE);
			if(m_nBorder > 1)
			{
				font.setDevice(m_oComposite, colors.TEXT_INACTIVE);
				layout.numColumns= 1;
				layout.marginWidth= m_nBorder;
				layout.marginHeight= m_nBorder;
				layout.marginLeft= 0;
				layout.marginRight= 0;
				layout.marginTop= 0;
				layout.marginBottom= 0;
				layout.horizontalSpacing= 0;
				layout.verticalSpacing= 0;
				m_oComposite.setLayout(layout);
				m_oComposite= new Composite(m_oComposite, SWT.NONE);
				layout= new GridLayout();				
			}
		}
		bHolder.value= true;
		newFont= font.defineNewColorObj(m_oComposite, bgcolor, colors.BACKGROUND, bHolder, layoutName);
		if(!HtmTags.tablestructure.equals(""))
		{
			BooleanHolder bStruct= new BooleanHolder();
			
			bStruct.value= true;
			structureFont= font.defineNewColorObj(m_oComposite, HtmTags.tablestructure, colors.BACKGROUND, bStruct, layoutName);
			structureFont.setDevice(m_oComposite);
			if(bStruct.value)
				structureFont.dispose();
			
		}else
			newFont.setDevice(m_oComposite);

		layout.numColumns= columns;
		layout.marginLeft= cellspacing;
		layout.marginRight= cellspacing;
		layout.marginTop= cellspacing;
		layout.marginBottom= cellspacing;
		layout.marginWidth= 0;
		layout.marginHeight= 0;
		layout.horizontalSpacing= cellspacing;
		layout.verticalSpacing= cellspacing;
		m_oComposite.setLayout(layout);
		for(HtmTags tag : m_lContent)
		{
			ContentRows row= (ContentRows)tag;
			
			row.execute(m_oComposite, newFont, classes);
		}
		if(bHolder.value)
			newFont.dispose();
	}
	
	/**
	 * whether a border should be set or not
	 * 
	 * @param set if the value is higher then null, it would set an border
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setBorder(int set)
	{
		m_nBorder= set;
	}
}
