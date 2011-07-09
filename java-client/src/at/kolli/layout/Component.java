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
import java.util.Map;
import java.util.Set;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Scale;
import org.eclipse.swt.widgets.Slider;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;

import org.apache.regexp.RE;

import at.kolli.automation.client.NoStopClientConnector;
import at.kolli.automation.client.NodeContainer;
import at.kolli.dialogs.DisplayAdapter;

/**
 * class representing all components which can displayed into the body-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Component extends HtmTags 
{
	/**
	 * enum type of layouts<br /
	 * <table>
	 *   <tr>
	 *     <td>
	 *       normal
	 *     </td>
	 *     <td>
	 *       component can be changed from user
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       readonly
	 *     </td>
	 *     <td>
	 *       Set the component to read only. Component can not change from user.
	 *       If this attribute is not set, but user have no permission to change,
	 *       the component is also set to read only as default
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       disabled
	 *     </td>
	 *     <td>
	 *       set the component to disabled. Component can not changed from user
	 *       and is grey deposited.
	 *       If this attribute is not set, but user have no permission to read,
	 *       the component is also set to disabled as default
	 *     </td>
	 *   </tr>
	 * </table>
	 */
	public enum layout	{	normal,
							readonly,
							disabled	}
	/**
	 * layout type from layout file if permission is correctly
	 * and permission on server is not checked
	 */
	private layout normal= layout.normal;
	/**
	 * disabled or read only attribute of component.<br />
	 */
	private layout actLayout= layout.normal;
	/**
	 * type attribute of component<br />
	 * button, togglebutton, checkbox, radio, text, spinner, slider, scale or combo
	 */
	public String type= "text";
	/**
	 * name attribute of component.<br />
	 * only for type radio, describe the group in which the radio button is
	 */
	public String name= "";
	/**
	 * value attribute of component.<br />
	 * for type radio or checkbox it describe the value which should be set
	 * type button or togglebutton the name inside the button
	 * type text the suffix behind the number
	 */
	public String value;
	/**
	 * width attribute of component.<br />
	 * the width of the components beside the types check and radio
	 * by the types slider or scale it shows an horizontal bar
	 * if the height is not set or lower
	 */
	public int width= -1;
	/**
	 * height attribute of component.<br />
	 * for type slider and scale it be shown an vertical bar
	 * if the width not be set or lower
	 */
	public int height= -1;
	/**
	 * size attribute of component<br />
	 * only for type COMBO
	 * describe how much rows should be displayed
	 * DEFAULT: 1
	 */
	public int size= 1;
	/**
	 * min attribute of component.<br />
	 * minimal-value for component slider and scale
	 */
	public int min= 0;
	/**
	 * max attribute of component.<br />
	 * maximal-value for component slider and scale
	 */
	public int max= 1500000;
	/**
	 * arrow attribute of component.<br />
	 * value for get higher or lower by pressing the arrow keys
	 * only for component slider
	 */
	public int arrowkey= 50;
	/**
	 * scroll attribute of component.<br />
	 * higher or lower value by clicking into the scroll bar
	 * for components slider and scale
	 */
	public int rollbarfield= 200;
	/**
	 * format attribute of component.<br />
	 * format of displayed number beginning with '#' than a number whitch
	 * representing the digits before decimal point and maby followd with an point
	 * for any decimal places or with an number how much decimal places should be showen
	 */
	public String format= "#1";
	/**
	 * how much digits after decimal point the component with type SPINNER have
	 */
	public int digits= 0;
	/**
	 * result attribute of component.<br />
	 * describing the position in the measure folders
	 * the subroutines only can be from type SWITCH for boolean or VALUE for numbers
	 * exp. 'folder:folder:subroutine'
	 */
	public String result= "";
	/**
	 * digits before decimal point. Calculated from format attribute.
	 */
	private int m_numBefore= 1;
	/**
	 * digits behind decimal point. Calculated from format attribute.
	 */
	private int m_numBehind= 0;
	/**
	 * variable is true if the result attribute on server is not reachable,
	 * incorrect or not set.
	 */
	private boolean m_bCorrectName= true;
	/**
	 * whether server has correct access to device for this component
	 */
	private boolean m_bDeviceAccess= true;
	/**
	 * actual value which was read from server
	 */
	private double m_nAktValue= 0;
	/**
	 * component as SWT widget
	 */
	private Control m_oComponent= null;
	/**
	 * Hash table for all entries inside the component with type COMBO
	 * from option-tags names for values
	 */
	private HashMap<String, Double> m_asComboValueEntrys= new HashMap<String, Double>();
	/**
	 * Hash table for all entries inside the component with type COMBO
	 * from option-tags as values for names
	 */
	private HashMap<Double, String> m_asComboNameEntrys= new HashMap<Double, String>();
	/**
	 * whether class have an listener
	 */
	private boolean haveListener= false;
	/**
	 * Listener {@link SWT}.MouseDown for component button
	 */
	private Listener m_eListener1;
	/**
	 * Listener {@link SWT}.MouseUp for component button
	 */
	private Listener m_eListener2;
	/**
	 * SelectionListener for components
	 */
	private SelectionListener m_eSelectionListener= null;
	
	/**
	 * create instance of all components whitch can display in the window
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public Component()
	{
		super("input");
	}
	
	/**
	 * method add entries to the component with type COMBO
	 * 
	 * @param entry name of entry witch will be displayed in the window
	 * @param value value of entry witch should be send to the server
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void addComboEntrys(String entry, double value)
	{
		m_asComboValueEntrys.put(entry, value);
		m_asComboNameEntrys.put(value, entry);
	}

	/**
	 * execute method to create the composite for display
	 * 
	 * @param composite parent composite
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite)
	{
		boolean disabled= false;
		boolean readonly= false;
		NoStopClientConnector client= NoStopClientConnector.instance();

		//System.out.println(type + " " + result);
		if(	actLayout.compareTo(layout.disabled) == 0 ||
			!m_bDeviceAccess								)
		{
			disabled= true;
		}
		if(actLayout.compareTo(layout.readonly) == 0)
			readonly= true;
		
		if(	this.type.equals("checkbox")
			||
			this.type.equals("radio")
			||
			this.type.equals("button")
			||
			this.type.equals("togglebutton")
			||
			this.type.equals("upbutton")
			||
			this.type.equals("leftbutton")
			||
			this.type.equals("rightbutton")
			||
			this.type.equals("downbutton")	)
		{
			int type= 0;
			Button button;
			GridData data= null;
			Double akt= client.getValue(this.result);

			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{	
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component " + this.type;
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			
			if(this.type.equals("checkbox"))
			{
				type= SWT.CHECK;
			}else if(this.type.equals("radio"))
			{
				type= SWT.RADIO;
			}else if(this.type.equals("button"))
			{
				type= SWT.PUSH;
			}else if(this.type.equals("togglebutton"))
			{
				type= SWT.TOGGLE;
			}else if(this.type.equals("upbutton"))
			{
				type= SWT.ARROW | SWT.UP;
			}else if(this.type.equals("leftbutton"))
			{
				type= SWT.ARROW | SWT.LEFT;
			}else if(this.type.equals("rightbutton"))
			{
				type= SWT.ARROW | SWT.RIGHT;
			}else if(this.type.equals("downbutton"))
			{
				type= SWT.ARROW | SWT.DOWN;
			}else
			{
				System.out.println("no defined input widget for type "+this.type);
				new Composite(composite, SWT.NONE);
				return;
			}
			
			disabled= disabled || readonly ? true : false;				
			button= new Button(composite, type);
			button.setEnabled(!disabled);
			if(width != -1)
			{
				data= new GridData();
				data.widthHint= width;
			}
			if(height != -1)
			{
				if(data == null)
					data= new GridData();
				data.heightHint= height;
			}
			if(data != null)
				button.setLayoutData(data);
			m_oComponent= button;
			if(	type == SWT.PUSH
				||
				type == SWT.TOGGLE	)
			{
				button.setText(this.value);
			}//else
				//button.setText(this.text);
			if(type == SWT.RADIO
				&&
				this.value.equals("0") 	)
			{
				button.setSelection(true);
			}
			
		}else if(this.type.equals("text"))
		{
			int style= readonly ? SWT.SINGLE | SWT.READ_ONLY : SWT.SINGLE;
			Text text= new Text(composite, style);
			GridData data= new GridData();
			Double akt= client.getValue(this.result);
			RE floatStr= new RE("([ +-/]|\\*|\\(|\\)|^)#([0-9])+(\\.([0-9])*)?([ +-/]|\\*|\\(|\\)|$)");
			
			// calculating the digits before decimal point
			// and after. save in m_numBefore and m_numBehind
			if(floatStr.match(this.format))
			{
				String v, b;
				
				try{
					v= floatStr.getParen(2);
					if(	v != null &&
						v.length() > 0	)
					{
						m_numBefore= Integer.parseInt(v);
					}
					b= floatStr.getParen(3);
					v= floatStr.getParen(4);
					if(	v != null &&
						v.length() > 0	)
					{
						m_numBehind= Integer.parseInt(v);
						
					}else if(	b != null &&
								b.equals(".")	)
					{
						m_numBehind= -1;
					}
					
				}catch(NumberFormatException ex)
				{
					// do nothing,
					// take defaultvalue from member
				}
			}

			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component input text";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			if(width == -1)
				width= 100;
			if(height != -1)
				data.heightHint= height;
			m_oComponent= text;
			data.widthHint= width;
			text.setLayoutData(data);
			text.setText(calculateInputValue(akt));
			text.setEnabled(!disabled);
			
		}else if(this.type.equals("slider"))
		{
			final Slider slider;
			GridData data= new GridData();
			int style= SWT.HORIZONTAL;
			Double akt= client.getValue(this.result);
			int value= this.min;

			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component slider";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}else
			{
				double d;
				
				if(akt == null)
					d= 0;
				else
					d= akt;				
				value= (int)d;
			}
			if(	width != -1
				||
				height != -1	)
			{
				if(	width == -1
					||
					width < height	)
				{
					style= SWT.VERTICAL;
					data.heightHint= height;
				}else
					data.widthHint= width;
			}else
				data.widthHint= 100;
			
			disabled= disabled || readonly ? true : false;
			slider= new Slider(composite, style);
			slider.setLayoutData(data);
			slider.setMinimum(min);
			slider.setMaximum(max);
			// Pfeiltaste
			slider.setIncrement(arrowkey);
			// klick auf Schieberegler
			slider.setPageIncrement(rollbarfield);
			// Aktuaelle Position
			slider.setSelection(value);
			slider.setEnabled(!disabled);
			m_oComponent= slider;
			
		}else if(this.type.equals("scale"))
		{
			Scale scale;
			GridData data= new GridData();
			int style= SWT.HORIZONTAL;
			Double akt= client.getValue(this.result);
			int value= this.min;

			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component scale";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}else
			{
				if(akt != null)
				{
					double d= akt;
					
					value= (int)d;
				}else
					value= 0;
			}
			
			if(	width != -1
				||
				height != -1	)
			{
				if(	width == -1
					||
					width < height	)
				{
					style= SWT.VERTICAL;
					data.heightHint= height;
				}else
					data.widthHint= width;
			}else
				data.widthHint= 100;

			disabled= disabled || readonly ? true : false;
			scale= new Scale(composite, style);
			scale.setLayoutData(data);
			scale.setMinimum(min);
			scale.setMaximum(max);
			// Pfeiltaste
			scale.setIncrement(arrowkey);
			// klick auf Schieberegler
			scale.setPageIncrement(rollbarfield);
			// Aktuaelle Position
			scale.setSelection(value);
			scale.setEnabled(!disabled);			
			m_oComponent= scale;
			m_nAktValue= (int)min;
			
		}else if(	this.type.equals("combo")
					&&
					this.size == 1				)
		{
			Combo combo= new Combo(composite, SWT.READ_ONLY | SWT.DROP_DOWN);
			GridData data= null;
			Double akt= client.getValue(this.result);
			double value= -1;
			
			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component combo";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			m_oComponent= combo;
			if(width != -1)
			{
				data= new GridData();
				data.widthHint= width;
			}
			if(height != -1)
			{
				if(data == null)
					data= new GridData();
				data.heightHint= height;
			}
			if(data != null)
				combo.setLayoutData(data);
			combo.setEnabled(!disabled);
			if(	!m_lContent.isEmpty())
			{
				boolean bfirst= true;
				
				for(HtmTags tag : m_lContent)
				{
					if(tag instanceof Option)
					{
						Option option= (Option)tag;
						String entry= option.getOptionString();
						
						if(option.value != null)
							value= option.value;
						else
							++value;
						addComboEntrys(entry, option.value);
						combo.add(entry);
						if(	(	akt == null &&
								bfirst == true	) ||
							(	akt != null &&
								akt.equals(m_asComboValueEntrys.get(entry))	)
																)
						{
							combo.setText(entry);
							bfirst= false;
						}
					}
				}
			}
			
		}else if(	this.type.equals("combo")
					&&
					this.size > 1				)
		{
			List list= new List(composite, m_lContent.size() > this.size ? SWT.SINGLE | SWT.V_SCROLL : SWT.SINGLE);
			GridData data= new GridData();
			Double akt= client.getValue(this.result);
			int item= 0;
			double value= -1;

			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component combo";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			m_oComponent= list;
			if(width != -1)
			{
				data= new GridData();
				data.widthHint= width;
			}
			data.heightHint= list.getItemHeight()  * this.size;
			//data.heightHint= (list.computeSize(SWT.DEFAULT, SWT.DEFAULT, true).x/4) * this.size;
			if(data != null)
				list.setLayoutData(data);
			list.setEnabled(!disabled);
			if(	!m_lContent.isEmpty()
				&&
				akt != null				)
			{
				for(HtmTags tag : m_lContent)
				{
					if(tag instanceof Option)
					{
						Option option= (Option)tag;
						String entry= option.getOptionString();
						
						if(option.value != null)
							value= option.value;
						else
							++value;
						addComboEntrys(entry, value);
						list.add(entry);
						//list.setBackground(SWT.COLOR_WHITE);
						if(akt == value)
							list.select(item);
						++item;
					}
				}
			}
			
		}else if(this.type.equals("spinner"))
		{
			int style= readonly ? SWT.READ_ONLY  : SWT.NONE;			
			final Spinner spinner= new Spinner(composite, style);
			GridData data= new GridData();
			Double akt= client.getValue(this.result);

			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component spinner";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			m_oComponent= spinner;
			if(width != -1)
			{
				data= new GridData();
				data.widthHint= width;
			}
			if(height != -1)
			{
				data.heightHint= height;
			}
			if(data != null)
				spinner.setLayoutData(data);
			spinner.setEnabled(!disabled);
			if(akt == null)
				akt= new Double(0);
			spinner.setValues((int)akt.doubleValue(), min, max, this.digits, arrowkey, rollbarfield);
			spinner.setEnabled(!disabled);
		}else
		{
			m_bCorrectName= false;
			if(HtmTags.debug)
				System.out.println("Component type '"+this.type+"' is no correct entry");
		}
	}
	
	/**
	 * disable or enable component whether server has correct access to device
	 * 
	 * @param subroutine string of subroutine as &lt;folder&gt;:&lt;subroutine&gt;
	 * @param enable whether server has access to device
	 * @return whether given subroutine was for this component
	 */
	public boolean setDeviceAccess(String subroutine, final boolean enable)
	{
		if(this.result.equals(subroutine))
		{
			final String type= this.type;
			
			m_bDeviceAccess= enable;
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					m_oComponent.setEnabled(enable);
					if(	type.equals("text")
						&&
						!enable						)
					{
						((Text)m_oComponent).setText(calculateInputValue(0.0));
					}
				}
			
			});
			return true;
		}
		return false;
	}
	
	/**
	 * set constant permission defined in layout file getting from server
	 * 
	 * @param perm permission layout from layout file
	 */
	public void setLayoutPermission(layout perm)
	{
		actLayout= perm;
		normal= perm;
	}
	/**
	 * setter routine to set actual permission inside of an tag
	 * 
	 * @param perm permission to set
	 */
	protected void setPermission(permission perm)
	{
		super.setPermission(perm);
		switch (perm)
		{
		case writeable:			
			if(	normal == layout.readonly ||
				normal == layout.disabled	)
			{
				actLayout= normal;
			}else
				actLayout= layout.normal;
			break;
			
		case readable:
			if(normal == layout.disabled)
				actLayout= normal;
			else
				actLayout= layout.readonly;
			
		case None:
		default:
			actLayout= layout.disabled;
			break;
		}
	}
	
	/**
	 * set the actual permission of the component
	 */
	public void setPermission()
	{
		Double res;
		NoStopClientConnector client;
		
		/*if(getPermission().equals(permission.None))
		{
			actLayout= noRead;
			return;
		}*/
		client= NoStopClientConnector.instance();
		res= client.getValue(result);
		if(!client.hasError())
		{
			if(	normal == layout.readonly ||
				!client.setValue(result, res)	)
			{
				setPermission(permission.readable);
				
			}else
				setPermission(permission.writeable);
				
		}else
		{
			setPermission(permission.None);
			if(client.getErrorCode().equals("ERROR 016"))
				m_bDeviceAccess= false;
		}
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
	public void addListeners()
	{
		final String result= this.result;
		final NoStopClientConnector client= NoStopClientConnector.instance();
		
		//System.out.println(type + " " + result);
		if(	m_bCorrectName
			&&
			getPermission().compareTo(permission.readable) >= 0
			&&
			client.haveSecondConnection()						)
		{
			client.hear(result);
		}
		if(	!m_bCorrectName
			||
			(	!getPermission().equals(permission.writeable)
				&&
				!this.type.equals("combo")					)	)
		{// if Component has no permission but type is an combo
		 // set listener to return back to the old value
			return;
		}
		
		haveListener= true;
		if(	this.type.equals("checkbox")
			||
			this.type.equals("radio")
			||
			this.type.equals("togglebutton")	)
		{
			String sValue= "0";
			if(this.type.equals("radio"))
				sValue= this.value;
			final String gType= this.type;
			final short gValue= Short.parseShort(sValue);
			
			((Button)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{
				public void widgetSelected(SelectionEvent ev)
				{
					short nValue;
					Button button= (Button)m_oComponent;
					boolean bSet= button.getSelection();
					
					if(gType.equals("radio"))
						nValue= gValue;
					else
					{
						if(bSet)
							nValue= 1;
						else
							nValue= 0;
					}
					client.setValue(result, nValue);
					if(client.hasError())
						System.out.println(client.getErrorMessage());
				}
			});
			
		}else if(	this.type.equals("button")
					||
					this.type.equals("upbutton")
					||
					this.type.equals("leftbutton")
					||
					this.type.equals("rightbutton")
					||
					this.type.equals("downbutton")	)
		{
			m_oComponent.addListener(SWT.MouseDown, m_eListener1= new Listener()
			{
			    public void handleEvent(final Event event)
			    {
			    	client.setValue(result, 1);
					if(client.hasError())
						System.out.println(client.getErrorMessage());
			    }
			});
			m_oComponent.addListener(SWT.MouseUp, m_eListener2= new Listener()
			{
			    public void handleEvent(final Event event)
			    {
			    	client.setValue(result, 0);
					if(client.hasError())
						System.out.println(client.getErrorMessage());
			    }
			}); 
			
		}else if(this.type.equals("text"))
		{
			RE floatStr= new RE("([ +-/]|\\*|\\(|\\)|^)#([0-9])+\\.([0-9])*([ +-/]|\\*|\\(|\\)|$)");
			String patternString= "[0-9]*";
			
			if(floatStr.match(this.format))
				patternString+= "(\\.[0-9]+)?";
			//patternString+= ").*";
			
			final RE pattern= new RE(patternString);
			
			((Text)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{
				@Override
				public void widgetDefaultSelected(SelectionEvent ev)
				{
					String string= ((Text)m_oComponent).getText();
					double value;
					
					if(pattern.match(string))
					{
						if(HtmTags.debug)
							System.out.println("user change text field " + name + " to " + string);
						string= pattern.getParen(0);
						if(!string.equals(""))
						{
							if(HtmTags.debug)
								System.out.println(" generate to number '" + string + "'");
							try{
								value= Double.parseDouble(string);
								
							}catch(NumberFormatException ex)
							{
								if(HtmTags.debug)
								{
									System.out.println("NumberFormatException for textfield " + name);
									System.out.println(" cannot convert value " + string);
									System.out.println();
								}
								value= 0;
							}
						}else
						{
							if(HtmTags.debug)
							{
								System.out.println("found no correct value for new string '" + string + "'");
								System.out.println("set value to 0");
							}
							value= 0;
						}
					}else
					{
						if(HtmTags.debug)
						{
							System.out.println("found no correct value for new string '" + string + "'");
							System.out.println("set value to 0");
						}
						value= 0;
					}
					client.setValue(result, value);
				}
			
			});
			
		}else if(this.type.equals("slider"))
		{
			((Slider)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					double value= ((Slider)m_oComponent).getSelection();
					
					client.setValue(result, value);
					super.widgetSelected(e);
				}
			
			});
			
		}else if(this.type.equals("scale"))
		{
			((Scale)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					double value= ((Scale)m_oComponent).getSelection();
					
					client.setValue(result, value);
					super.widgetSelected(e);
				}
			
			});
			
		}else if(	this.type.equals("combo")
					&&
					this.size == 1				)
		{
			final permission perm= getPermission();
			
			((Combo)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter() 
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					String key= ((Combo)m_oComponent).getText();
					Double value= m_asComboValueEntrys.get(key);
					
					if(perm.equals(permission.readable))
					{
						((Combo)m_oComponent).setText(m_asComboNameEntrys.get(m_nAktValue));
					}else
						client.setValue(result, value);
					super.widgetSelected(e);
				}			
			});
			
		}else if(	this.type.equals("combo")
				&&
				this.size > 1					)
		{
			final permission perm= getPermission();
			
			((List)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter() 
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{					
					String key[]= ((List)m_oComponent).getSelection();
					Double value= m_asComboValueEntrys.get(key[0]);

					if(perm.equals(permission.readable))
					{
						((List)m_oComponent).setSelection((int)m_nAktValue);
					}else
						client.setValue(result, value);
					super.widgetSelected(e);
				}
			
			});
			
		}else if(this.type.equals("spinner"))
		{
			((Spinner)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{		
				@Override
				public void widgetSelected(SelectionEvent e)
				{						
					int value= ((Spinner)m_oComponent).getSelection();
					
					client.setValue(result, value);
					super.widgetSelected(e);
				}			
			});
		}
	}
	
	/**
	 * calculate the string value for an component with type text
	 * 
	 * @param value the value from server which should displayed
	 * @return value as string with observance the format attribute and add the suffix (attribute value)
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	protected String calculateInputValue(Double value)
	{
		boolean bInt= false;
		String stringValue= "" + value;
		int strPos= 0;
		char stringChars[]= stringValue.toCharArray();
		String Rv= "";
		
		if(!m_bDeviceAccess)
			return "ERROR";
		else if(getPermission().equals(permission.None))
			return " --- ";
		while((	stringChars.length > strPos
				&&
				stringChars[strPos] != '.'	))
		{
			Rv+= stringChars[strPos];
			++strPos;
		}
		Double sh= new Double(stringValue.substring(0, strPos));
		if(value.equals(sh))
			bInt= true;
		for(int c= strPos; c<m_numBefore; ++c)
			Rv= "0" + Rv;
		if(	m_numBehind > 0 ||
			m_numBehind == -1	)
		{
			int behind= m_numBehind;

			if(	m_numBehind >0 ||
				!bInt				)
			{
				++strPos;
				if(m_numBehind == -1)
					behind= stringValue.length() - strPos;
				Rv+= ".";
				for(int c= 0; c<behind; ++c)
				{
					if(stringChars.length > strPos)
					{
						Rv+= stringChars[strPos];
						++strPos;
					}else
						Rv+= "0";
				}
			}
		}
		return Rv+= " " + this.value;
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
		if(	!m_bCorrectName
			||
			!haveListener
			||
			(	!getPermission().equals(permission.writeable)
				&&
				!this.type.equals("combo")					)	)
		{
			return;
		}
		
		haveListener= false;
		if(	this.type.equals("checkbox")
			||
			this.type.equals("radio")
			||
			this.type.equals("togglebutton")	)
		{
			((Button)m_oComponent).removeSelectionListener(m_eSelectionListener);
			m_eSelectionListener= null;
			
		}else if(	this.type.equals("button")
					||
					this.type.equals("upbutton")
					||
					this.type.equals("leftbutton")
					||
					this.type.equals("rightbutton")
					||
					this.type.equals("downbutton")	)
		{
			m_oComponent.removeListener(SWT.MouseDown, m_eListener1);
			m_oComponent.removeListener(SWT.MouseUp, m_eListener2); 
			m_eListener1= null;
			m_eListener2= null;
			
		}else if(this.type.equals("text"))
		{
			((Text)m_oComponent).removeSelectionListener(m_eSelectionListener);
			//((Text)m_oComponent).removeVerifyListener(m_eVerifyListener);
			
		}else if(this.type.equals("slider"))
		{
			((Slider)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(this.type.equals("scale"))
		{
			((Scale)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(	this.type.equals("combo")
					&&
					this.size == 1				)
		{
			((Combo)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(	this.type.equals("combo")
				&&
				this.size > 1					)
		{
			((List)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(this.type.equals("spinner"))
		{
			((Spinner)m_oComponent).removeSelectionListener(m_eSelectionListener);
		}
	}
	
	/**
	 * gets the component as SWT-widget.<br />
	 * This method is for an faster access to the components.
	 * 
	 * @return component as SWT-widget
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public Control getComponentWidget()
	{
		return m_oComponent;
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
	protected void insert(HtmTags newTag)
	{
		m_lContent.add(newTag);
		newTag.m_oParent= this;
	}
	
	/**
	 * method look on server whether value of component is changed.
	 * 
	 * @param results if attribute result not in this map ask value from server and method save the result in the map
	 * @return false if the value is not changed or the result attribute not reachable, otherwise true
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	private boolean setNewValue(final Map<String, Double> results)
	{
		NoStopClientConnector client;
		Double get= results.get(this.result);
		
		if(get == null)
		{
			client= NoStopClientConnector.instance();
			get= client.getValue(this.result);
			if(get == null)
			{
				System.out.println(client.getErrorMessage());
				System.out.println("set component " + this.tagName + " with type " + this.type + " to false");
				m_bCorrectName= false;
				return false;
			}
			results.put(this.result, get);
		}
		if(m_nAktValue == get)
			return false;
		m_nAktValue= get;
		return true;
	}
	
	/**
	 * method listen on server whether value of component is changed
	 * 
	 * @param results map of result attributes with actual values
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void serverListener(final Map<String, Double> results, NodeContainer cont)
	{
		boolean bValue= false;
		
		if(	!m_bCorrectName
			||
			getPermission().equals(permission.None) 	)
		{
			return;
		}
		if(	cont != null
			&&
			cont.hasValue()	)
		{
			if(!(cont.getFolderName() + ":" + cont.getSubroutineName()).equals(this.result))
				return;
			if(cont.hasDoubleValue())
			{
				m_nAktValue= cont.getDValue();
				bValue= true;
				
			}else if(cont.hasStringValue())
			{
				boolean bAccess= true;
				String value= cont.getSValue();
				
				if(value.equals("access"))
				{
					bAccess= true;
					bValue= true;
				}else if(value.equals("noaccess"))
				{
					bAccess= false;
					bValue= true;
				}
				if(bValue)
					setDeviceAccess(this.result, bAccess);
			}
		}
		if(!bValue)
		{
			if(!setNewValue(results))
				return;
		}
		
		if(	this.type.equals("checkbox")
			||
			this.type.equals("togglebutton")	)
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Button button= (Button)m_oComponent;
					boolean bSet= m_nAktValue == 0 ? false : true;
					
					button.setSelection(bSet);
				}
			
			});
			
		}else if(this.type.equals("radio"))
		{
			final double nValue= Double.parseDouble(this.value);
			
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Button button= (Button)m_oComponent;
					boolean bSet= m_nAktValue == nValue ? true : false;
					
					button.setSelection(bSet);
				}
			
			});
			
		//}else if(this.type.equals("text"))
		}else if(m_oComponent instanceof Text)
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					//String string= new String(m_nAktValue + " " + suffix);
					Text text= (Text)m_oComponent;
					
					text.setText(calculateInputValue(m_nAktValue));
				}
			
			});
			
		}else if(this.type.equals("slider"))
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Slider slider= (Slider)m_oComponent;
					
					slider.setSelection((int)m_nAktValue);
				}
			
			});
			
		}else if(this.type.equals("scale"))
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Scale scale= (Scale)m_oComponent;
					
					scale.setSelection((int)m_nAktValue);
				}
			
			});
		}else if(m_oComponent instanceof Combo)
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Set<String> entrys;
					Combo combo= (Combo)m_oComponent;
					
					entrys= m_asComboValueEntrys.keySet();
					for(String entry : entrys)
					{
						Double akt= m_asComboValueEntrys.get(entry);
						if(akt.equals(m_nAktValue))
						{
							combo.setText(entry);
							break;
						}
					}
				}
			
			});
		}else if(m_oComponent instanceof List)
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					List list= (List)m_oComponent;
					
					list.select((int)m_nAktValue);
				}
			
			});
		}else if(m_oComponent instanceof Spinner)
		{
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Spinner spinner= (Spinner)m_oComponent;
					
					spinner.setSelection((int)m_nAktValue);
				}
			
			});
		}
	}
}
