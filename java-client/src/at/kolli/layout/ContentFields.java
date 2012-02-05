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
import java.util.Map;

import org.eclipse.swt.SWT;
import org.eclipse.swt.browser.Browser;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;

import at.kolli.automation.client.MsgClientConnector;
import at.kolli.automation.client.NodeContainer;

/**
 * class representing an field (column) in an table for layout on display
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class ContentFields extends HtmTags implements IComponentListener
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
	 * this field or body is an internet browser
	 */
	public String href= "";
	/**
	 * variable be filled with the actual internet address
	 * when side with browser will be leave.<br />
	 * This will be only done when an soft button be defined.
	 */
	public String m_sActRef= "";
	/**
	 * browser object for href
	 */
	private Browser m_oBrowser= null;
	
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
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, HashMap<String, HtmTags> classes) throws IOException
	{
		int count;
		ArrayList<Integer > lenList= new ArrayList<Integer>();
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
		
		if(	this instanceof Body &&
			href != ""				)
		{
			m_oBrowser= new Browser(mainCp, SWT.NONE);			
			composite.setLayout(new FillLayout());
			mainCp.setLayout(new FillLayout());
			this.composite= mainCp;
			return;
		}
		if(	m_lContent.size() == 0 &&
			href == "")
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
		if(href.equals(""))
		{
			for(HtmTags ntag : m_lContent)
			{
				if(ntag instanceof Break)
				{
					lenList.add(count);
					count= 0;
				}else
					++count;
			}
		}else
			++count;
		lenList.add(count);
		rowLayout.numColumns= lenList.get(0);
		count= 1; //<- for next list index
		
		
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
		
		if(!href.equals(""))
		{
			final int minus= 10;
			Composite gridCompo= new Group(rowCp, SWT.SHADOW_ETCHED_IN);//new Composite(rowCp, SWT.NONE);
			GridLayout gridLayout= new GridLayout();
			//Composite browseCompo= new Composite(gridCompo, SWT.NONE); 
			GridData data= null;

			m_oBrowser= new Browser(gridCompo, SWT.NONE);
			if(	width != -1 &&
				width > minus	)
			{
				data= new GridData();
				data.widthHint= width - minus;
			}
			if(	height != -1 &&
				height > minus	)
			{
				if(data == null)
					data= new GridData();
				data.heightHint= height - minus;
			}
			if(data != null)
				gridCompo.setLayoutData(data);
			gridLayout.marginWidth= 1;
			gridLayout.marginHeight= 0;
			gridCompo.setLayout(new FillLayout());
			this.composite= gridCompo;
			//tag.execute(gridCompo);
			
			//composite.setLayout(new FillLayout());
			//browser.setLayout(new FillLayout());
			return;
		}else
		{
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
					rowLayout.numColumns= lenList.get(count);
					++count; //<- for next list index
					rowCp.setLayout(rowLayout);
					rowCp.setLayoutData(rowData);
				}else
				{
					Composite gridCompo= new Composite(rowCp, SWT.NONE);
					GridLayout gridLayout= new GridLayout();
					
					gridLayout.marginWidth= 1;
					gridLayout.marginHeight= 0;
					gridCompo.setLayout(gridLayout);
					tag.execute(gridCompo, classes);
				}
			}
		}
	}

	/**
	 * method listen on server whether value of component is changed
	 * 
	 * @param results map of result attributes with actual values
	 * @param cont container with new value on which result of folder and subroutine
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void serverListener(final Map<String, Double> results, NodeContainer cont) throws IOException
	{
		// nothing to do
	}

	/**
	 * get exist browser object
	 * 
	 * @return browser object
	 */
	public Browser getBrowser()
	{
		return m_oBrowser;
	}
	
	/**
	 * add listeners if the component have an correct result attribute.<br />
	 * This method is to listen on activity if the component is in an {@link Composite}
	 * witch is be on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void addListeners() throws IOException
	{
		if(!href.equals(""))
		{
			Browser browser= getBrowser();
			String resource= browser.getUrl();
			
			if(HtmTags.debug)
				System.out.println("current resource of browser is '" + resource + "'");
			if(	resource.equals("") ||
				resource.equals("about:blank")	)
			{
				String value;
				
				if(m_sActRef.equals(""))
				{
					value= href;
					
				}else
					value= m_sActRef;	
				if(HtmTags.debug)
					System.out.println("set resource to '" + value + "'");
				browser.setUrl(value);
				
			}
		}
	}

	/**
	 * remove listeners if the component have an correct result attribute.<br />
	 * This method is to remove all listeners which set bevore with addListeners()
	 * if the component is in an {@link Composite} witch is not on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void removeListeners()
	{
		if(!href.equals(""))
		{
			String value;
			
			value= m_oBrowser.getUrl();
			System.out.println("set browser to blank screen");
			m_oBrowser.setUrl("about:blank");
			if(	!value.equals("") &&
				!value.equals("about:blank")	)
			{
				m_sActRef= value;
			}
			System.gc();
		}
	}
}
