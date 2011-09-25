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

import org.eclipse.swt.SWT;
import org.eclipse.swt.browser.Browser;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;

import at.kolli.automation.client.MsgClientConnector;

/**
 * class representing an field (column) in an table for layout on display
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class ContentFields extends HtmTags
{
	/**
	 * {@link Composite} for display the inherited widgets
	 */
	public Composite composite;
	/**
	 * how much fields in the row this field will be need
	 */
	public int colspan= 1;
	/**
	 * how much fields in the column this field will be need
	 */
	public int rowspan= 1;
	/**
	 * minimal width of field
	 */
	public int width= -1;
	/**
	 * minimal height of field
	 */
	public int height= -1;
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
	 * group name for permission
	 */
	public String permgroup= "";
	/**
	 * whether the user want to see an border by all composites<br />
	 * in this case it will be create an  {@link Group}
	 */
	private boolean m_bBorder= false;
	/**
	 * if this variable filled,
	 * this field or body as internet browser
	 */
	public String href= "";
	
	/**
	 * create instance of td-tag
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public ContentFields()
	{
		super("td");
	}

	/**
	 * create instance of extended tag
	 * 
	 * @param name name of tag
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public ContentFields(String name)
	{
		super(name);
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
		if(this instanceof Body)
			newTag.m_oParent= this;
		m_lContent.add(newTag);
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
	 * method to generate the widget in the display window
	 * 
	 * @param composite parent {@link Composite}
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite) throws IOException
	{
		int count;
		Composite mainCp;
		Composite fieldCp;
		Composite rowCp;
		GridLayout mainLayout= new GridLayout();
		GridLayout fieldLayout= new GridLayout();
		GridLayout rowLayout= new GridLayout();
		GridData mainData= new GridData();
		GridData fieldData= new GridData();
		GridData rowData= new GridData();

		askPermission();
		if(getPermission().compareTo(permission.readable) == -1)
			return;
		if(	m_bBorder && 
			(	m_lContent.size() > 0 ||
				href != ""				)	)
		{
			mainCp= new Group(composite, SWT.SHADOW_ETCHED_IN);
			
		}else
			mainCp= new Composite(composite, SWT.NONE);
		
		if(href != "")
		{
			Browser browser= new Browser(mainCp, SWT.NONE);
			
			composite.setLayout(new FillLayout());
			mainCp.setLayout(new FillLayout());
			browser.setUrl(href);
			return;
		}
		if(m_lContent.size() == 0)
		{// no content exist
			return;
		}
		fieldCp= new Composite(mainCp, SWT.NONE);
		//fieldCp= new Group(mainCp, SWT.SHADOW_ETCHED_IN);
		rowCp= new Composite(fieldCp, SWT.NONE);
		//rowCp= new Group(fieldCp, SWT.SHADOW_ETCHED_IN);	
		
		
		mainLayout.numColumns= 1;
		mainLayout.marginWidth= cellpadding;
		mainLayout.marginHeight= cellpadding;

		fieldLayout.marginHeight= 0;
		fieldLayout.marginWidth= 0;
		
		rowLayout.marginHeight= 0;
		rowLayout.marginWidth= 0;
		rowLayout.marginTop= 0;
		rowLayout.marginBottom= 0;
		count= 0;
		for(HtmTags tag : m_lContent)
		{
			if(tag instanceof Break)
				break;
			++count;
		}
		rowLayout.numColumns= count;
		
		
		mainCp.setLayout(mainLayout);
		fieldCp.setLayout(fieldLayout);
		rowCp.setLayout(rowLayout);

		mainData.horizontalSpan= colspan;
		mainData.horizontalAlignment= GridData.FILL;
		mainData.verticalSpan= rowspan;
		mainData.verticalAlignment= GridData.FILL;
		if(this.width != -1)
			mainData.widthHint= this.width;
		if(rowspan == 1)
		{
			if(this.height != -1)
				mainData.heightHint= this.height;
		}

		fieldData.horizontalAlignment= align;
		fieldData.grabExcessHorizontalSpace= true;
		fieldData.verticalAlignment= valign;
		fieldData.grabExcessVerticalSpace= true;

		rowData.horizontalAlignment= align;
		rowData.grabExcessHorizontalSpace= true;
		
		
		mainCp.setLayoutData(mainData);
		fieldCp.setLayoutData(fieldData);
		rowCp.setLayoutData(rowData);

		for(HtmTags tag : m_lContent)
		{
			if(tag instanceof Break)
			{
				rowCp= new Composite(fieldCp, SWT.NONE);
				//rowCp= new Group(fieldCp, SWT.SHADOW_NONE);
				rowLayout= new GridLayout();
				rowData= new GridData();
				
				rowData.horizontalAlignment= align;
				rowData.grabExcessHorizontalSpace= true;
				rowLayout.marginTop= 0;
				rowLayout.marginBottom= 0;
				rowLayout.marginHeight= 0;
				rowLayout.marginWidth= 0;
				count= 0;
				for(HtmTags ntag : m_lContent)
				{
					if(ntag instanceof Break)
						break;
					++count;
				}
				rowLayout.numColumns= count;
				rowCp.setLayout(rowLayout);
				rowCp.setLayoutData(rowData);
			}else
			{
				Composite gridCompo= new Composite(rowCp, SWT.NONE);
				GridLayout gridLayout= new GridLayout();
				
				gridLayout.marginWidth= 1;
				gridLayout.marginHeight= 0;
				gridCompo.setLayout(gridLayout);
				tag.execute(gridCompo);
			}
		}
	}
}
