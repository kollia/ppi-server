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
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowLayout;
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
		Composite cp;
		Composite outCp;
		Composite inCp;
		GridLayout oLayout= new GridLayout();
		GridData data= new GridData();
		RowLayout outLayout= new RowLayout();
		RowLayout inLayout= new RowLayout();

		askPermission();
		if(getPermission().compareTo(permission.readable) == -1)
			return;
		if(m_bBorder)
			cp= new Group(composite, SWT.SHADOW_NONE);
		else
			cp= new Composite(composite, SWT.NONE);
		outCp= new Composite(cp, SWT.NONE);
		//inCp= new Group(outCp, SWT.SHADOW_NONE);
		inCp= new Composite(outCp, SWT.NONE);
		/*int count= 0;		
		for(ArrayList<HtmTags> list : m_lRows)
		{
			int c= list.size();
			
			if(c > count)
				count= c;
		}*/
		data.horizontalSpan= colspan;
		data.verticalSpan= rowspan;
		data.horizontalAlignment= align;
		data.verticalAlignment= valign;
		if(this.width != -1)
			data.minimumWidth= this.width;
		if(this.height != -1)
			data.minimumHeight= this.height;
		
		oLayout.numColumns= 1;
		oLayout.marginWidth= cellpadding;
		oLayout.marginHeight= cellpadding;

		outLayout.type= SWT.VERTICAL;
		outLayout.wrap= false;
		outLayout.spacing= 0;
		outLayout.marginHeight= 0;
		outLayout.marginWidth= 0;
		
		inLayout.type= SWT.HORIZONTAL;
		inLayout.wrap= false;
		inLayout.spacing= 0;
		/*inLayout.marginTop= 0;
		inLayout.marginBottom= 0;
		inLayout.marginLeft= 0;
		inLayout.marginRight= 0;
		inLayout.marginHeight= 0;
		inLayout.marginWidth= 0;*/

		cp.setLayoutData(data);
		cp.setLayout(oLayout);
		outCp.setLayout(outLayout);
		inCp.setLayout(inLayout);

		if(m_lContent.size() == 0)
		{
			/*data.heightHint= 10;
			data.widthHint= 10;*/
			//inCp.setLayout(inLayout);
			//outCp.setLayout(outLayout);
			//outCp.setLayoutData(new RowData(5, 5));
			//inCp.setLayoutData(new RowData(100, 20));
			return;
		}
		//for(ArrayList<HtmTags> list : m_lRows)
		//{
			for(HtmTags tag : m_lContent)
			{
				if(tag instanceof Break)
				{
					inCp= new Composite(outCp, SWT.NONE);
					//inCp= new Group(outCp, SWT.SHADOW_NONE);
					inCp.setLayout(inLayout);
				}else
				{
					Composite gridCompo= new Composite(inCp, SWT.NONE);
					GridLayout gridLayout= new GridLayout();
					
					//gridLayout.numColumns= 1;
					gridLayout.marginWidth= 0;
					gridLayout.marginHeight= 0;
					/*gridLayout.marginLeft= 0;
					gridLayout.marginTop= 0;
					gridLayout.marginRight= 0;
					gridLayout.marginBottom= 0;*/
					gridCompo.setLayout(gridLayout);
					tag.execute(gridCompo);
				}
			}
		//}
	}
}
