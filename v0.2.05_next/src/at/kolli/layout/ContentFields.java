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
import java.util.LinkedList;
import java.util.Map;

import javax.swing.text.AbstractDocument.BranchElement;

import org.eclipse.swt.SWT;
import org.eclipse.swt.browser.Browser;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.omg.CORBA.BooleanHolder;

import at.kolli.automation.client.MsgClientConnector;
import at.kolli.automation.client.NodeContainer;
import at.kolli.layout.FontObject.colors;

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
	 * group name for permission
	 */
	public String permgroup= "";
	/**
	 * color of table field or body
	 */
	public String bgcolor= "";
	/**
	 * whether the user want to see an border by all composites<br />
	 * in this case it will be create an  {@link Group}
	 */
	private int m_nBorder= 0;
	/**
	 * content name of fieldset
	 */
	public String fieldset= "";
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
		if(	this instanceof Body ||
			this instanceof FieldSet	)
		{
			newTag.m_oParent= this;
		}
		//else
		//	System.out.print("");
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
	public void setBorder(int set)
	{
		m_nBorder= set;
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
	 * method to generate widgets in the display window
	 * 
	 * @param composite parent {@link Composite}
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 08.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException
	{
		/**
		 * whether last tag was an break tag
		 */
		BooleanHolder bBrLast= new BooleanHolder();
		/**
		 * whether font for mainCp was set for legend
		 */
		boolean bSetFontMain= false;
		int count;
		LinkedList<Integer > lenList= new LinkedList<Integer>();
		Composite mainCp;
		Composite rowCp;
		Composite fieldCp;
		GridLayout mainLayout= new GridLayout();
		GridLayout rowLayout= new GridLayout();
		GridLayout fieldLayout= new GridLayout();
		GridData mainData= new GridData();
		GridData rowData= new GridData();
		GridData fieldData= new GridData();
		FontObject newFont= font;
		FontObject legendFont;
		BooleanHolder bHolder= new BooleanHolder();

		if(align == -1)
			align= GridData.BEGINNING;
		if(valign == -1)
			valign= GridData.CENTER;
		askPermission();
		if(getPermission().compareTo(permission.readable) == -1)
			return;
		
		if(	this instanceof FieldSet ||
			(	m_nBorder == 1 && 
				(	m_lContent.size() > 0 ||
					href != ""				)	)	)
		{
			mainCp= new Group(composite, SWT.SHADOW_ETCHED_IN);
			
		}else
		{
			mainCp= new Composite(composite, SWT.NONE);	
		}
		
		if(	this instanceof FieldSet &&
			m_lContent.size() > 0 &&
			m_lContent.get(0) instanceof Legend	)
		{
			Legend legend= (Legend)m_lContent.get(0);
		
			bHolder.value= true;
			legendFont= legend.getFontObject(mainCp, font, bHolder);
			((Group)mainCp).setText(legend.getText());
			legendFont.setDevice(mainCp);
			if(bHolder.value)
				legendFont.dispose();
			bSetFontMain= true;
		}

		bHolder.value= true;
		newFont= font.defineNewColorObj(mainCp, bgcolor, colors.BACKGROUND, bHolder, layoutName);
		
		if(this instanceof Body)
		{	
			if(!href.equals(""))
			{		
				composite.setLayout(new FillLayout());
				m_oBrowser= new Browser(mainCp, SWT.NONE);
				mainCp.setLayout(new FillLayout());
				this.composite= mainCp;
				return;
			}
			newFont.setDevice(composite);
		}
		if(	m_lContent.size() == 0 &&
			href.equals("")				)
		{// no content exist
			Label label= new Label();
			
			label.setText(" ");
			m_lContent.add(label);
		}
		rowCp= new Composite(mainCp, SWT.NONE);
		fieldCp= new Composite(rowCp, SWT.NONE);
		
		if(!bSetFontMain)
			newFont.setDevice(mainCp);
		newFont.setDevice(rowCp);
		newFont.setDevice(fieldCp);
		

		if(this instanceof Body)
			cellpadding= 0;
		mainLayout.numColumns= 1;
		mainLayout.marginWidth= 0;
		mainLayout.marginHeight= 0;
		mainLayout.marginLeft= cellpadding;
		mainLayout.marginRight= cellpadding;
		mainLayout.marginTop= cellpadding;
		mainLayout.marginBottom= cellpadding;
		mainLayout.horizontalSpacing= cellpadding;
		mainLayout.verticalSpacing= cellpadding;
		
		rowLayout.marginHeight= 0;
		rowLayout.marginWidth= 0;
		rowLayout.marginLeft= 0;
		rowLayout.marginRight= 0;
		rowLayout.marginTop= 0;
		rowLayout.marginBottom= 0;
		rowLayout.horizontalSpacing= 0;
		rowLayout.verticalSpacing= 0;
		
		fieldLayout.marginHeight= 0;
		fieldLayout.marginWidth= 0;
		fieldLayout.marginLeft= 0;
		fieldLayout.marginRight= 0;
		fieldLayout.marginTop= 0;
		fieldLayout.marginBottom= 0;
		fieldLayout.horizontalSpacing= 0;
		fieldLayout.verticalSpacing= 0;
		count= 0;
		if(href.equals(""))
		{
			// count how much entries of text
			// the field has
			int o= 0;
			HtmTags ntag;
			LinkedList<Integer> lRv;
			
			bBrLast.value= false;
			while(o < m_lContent.size())
			{
				ntag= m_lContent.get(o);
				if(ntag instanceof Break)
				{
					if(bBrLast.value)
					{
						Label lable= new Label();
						
						lable.setText(" ");
						m_lContent.add(o, lable);
						++o;
						++count;
					}
					lenList.add(count);
					count= 0;
					bBrLast.value= true;
					
				}else if(ntag instanceof Style)
				{
					lRv= ntag.textFields(count, bBrLast);
					for(int i= 0; i < lRv.size()-1; ++i)
					{
						lenList.addLast(lRv.get(i));
						//if(lRv.get(i) > maxVal)
						//	maxVal= lRv.get(i);
					}
					count= lRv.get(lRv.size()-1);
						
				}else
				{
					bBrLast.value= false;
					++count;
				}
				++o;
			}
		}else
			++count;
		lenList.add(count);
		fieldLayout.numColumns= lenList.getFirst();
		lenList.removeFirst(); //<- for next list index
		
		
		mainCp.setLayout(mainLayout);
		rowCp.setLayout(rowLayout);
		fieldCp.setLayout(fieldLayout);

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

		rowData.horizontalAlignment= GridData.FILL;
		rowData.grabExcessHorizontalSpace= true;
		rowData.verticalAlignment= valign;
		rowData.grabExcessVerticalSpace= true;

		fieldData.horizontalAlignment= align;
		fieldData.grabExcessHorizontalSpace= true;
		//fieldData.verticalAlignment= GridData.END;
		//fieldData.grabExcessVerticalSpace= true;
		
		
		mainCp.setLayoutData(mainData);
		rowCp.setLayoutData(rowData);
		fieldCp.setLayoutData(fieldData);
		
		if(!href.equals(""))
		{
			final int minus= 10;
			Composite gridCompo= new Group(fieldCp, SWT.SHADOW_ETCHED_IN);//new Composite(fieldCp, SWT.NONE);
			GridLayout gridLayout= new GridLayout();
			//Composite browseCompo= new Composite(gridCompo, SWT.NONE); 
			GridData data= null;

			newFont.setDevice(gridCompo);
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
			if(bHolder.value)
				newFont.dispose();
			return;
		}else
		{	
			for(HtmTags tag : m_lContent)
			{
				if(tag instanceof Break)
				{
					fieldCp= new Composite(rowCp, SWT.NONE);
					fieldLayout= new GridLayout();
					fieldData= new GridData();
					
					newFont.setDevice(fieldCp);
					//fieldFont.setDevice(fieldCp);
					fieldData.horizontalAlignment= align;
					fieldData.grabExcessHorizontalSpace= true;
					fieldLayout.marginHeight= 0;
					fieldLayout.marginWidth= 0;
					fieldLayout.marginLeft= 1;
					fieldLayout.marginRight= 1;
					fieldLayout.marginTop= 1;
					fieldLayout.marginBottom= 1;
					fieldLayout.horizontalSpacing= 0;
					fieldLayout.verticalSpacing= 0;
					fieldLayout.numColumns= lenList.getFirst();
					lenList.removeFirst(); //<- for next list index
					fieldCp.setLayout(fieldLayout);
					fieldCp.setLayoutData(fieldData);
				}else
				{
					if(tag instanceof Style)
					{
						((Style)tag).align= align;
						((Style)tag).valign= valign;
						fieldCp= ((Style)tag).execute(rowCp, fieldCp, newFont, classes, lenList);
					}else
						tag.execute(fieldCp, newFont, classes);
				}
			}
		}
		if(bHolder.value)
			newFont.dispose();			
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
