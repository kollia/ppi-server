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

import java.awt.font.TextLayout;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.Map;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.MouseMoveListener;
import org.eclipse.swt.events.MouseTrackListener;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.FontData;
import org.eclipse.swt.graphics.FontMetrics;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.ImageData;
import org.eclipse.swt.graphics.ImageLoader;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.graphics.Region;
import org.eclipse.swt.graphics.TextStyle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Slider;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;
import org.omg.CORBA.BooleanHolder;

import at.kolli.automation.client.MsgTranslator;

/**
 * object to create fonts and colors
 * 
 * @author Alexander Kolli
 *
 */
public class FontObject
{

	/**
	 * all defined colors inside object
	 * 
	 * @author Alexander Kolli
	 */
	enum colors
	{
		TEXT,
		TEXT_INACTIVE,
		BACKGROUND,
		
		LIST_SELECTED_TEXT,
		LIST_SELECTED_ARRAY,
		LIST_BACKGROUND,
		
		WIDGET,
		WIDGET_BORDER,
		WIDGET_BORDER_SHADOW,
		WIDGET_HOVER,
		WIDGET_BORDER_HOVER,
		WIDGET_BORDER_SHADOW_HOVER,
		WIDGET_PRESSED,
		WIDGET_BORDER_PRESSED,
		WIDGET_BORDER_SHADOW_PRESSED,
		WIDGET_INACTIVE,
		WIDGET_BORDER_INACTIVE,
		WIDGET_BORDER_SHADOW_INACTIVE,
		WIDGET_FOCUS
	}
	/**
	 * all equivalent SWT color to FontObject colors
	 */
	private static int[] colorIds = new int[] {
		SWT.COLOR_WIDGET_FOREGROUND, SWT.COLOR_TITLE_INACTIVE_FOREGROUND, SWT.COLOR_WIDGET_BACKGROUND,		
		SWT.COLOR_LIST_SELECTION_TEXT, SWT.COLOR_LIST_SELECTION, SWT.COLOR_LIST_BACKGROUND,		
		SWT.COLOR_WIDGET_LIGHT_SHADOW, SWT.COLOR_WIDGET_BORDER, SWT.COLOR_WIDGET_HIGHLIGHT_SHADOW,
		SWT.COLOR_WIDGET_HIGHLIGHT_SHADOW, SWT.COLOR_TITLE_BACKGROUND_GRADIENT, SWT.COLOR_TITLE_BACKGROUND_GRADIENT,
		SWT.COLOR_WIDGET_DARK_SHADOW, SWT.NONE, SWT.NONE,
		SWT.COLOR_WIDGET_NORMAL_SHADOW, SWT.NONE, SWT.NONE,
		SWT.COLOR_TITLE_BACKGROUND,
		
// all other not needed SWT colors
		SWT.COLOR_INFO_BACKGROUND, SWT.COLOR_INFO_FOREGROUND,
		SWT.COLOR_LIST_FOREGROUND,		 
		SWT.COLOR_TITLE_FOREGROUND,
		SWT.COLOR_TITLE_INACTIVE_BACKGROUND_GRADIENT,
		SWT.COLOR_TITLE_INACTIVE_FOREGROUND				};
	/**
	 * list of all color objects
	 */
	private LinkedList<Color> m_lColors= new LinkedList<Color>();
	/**
	 * list of color names defined inside m_lColors
	 */
	private LinkedList<String> m_lsColors= new LinkedList<String>();
	/**
	 * new defined color object for dispose
	 */
	private ArrayList<Color> m_aDefinedColors= new ArrayList<Color>();
	/**
	 * whether all colors of object defined 
	 */
	private boolean bInit= false;
	/**
	 * character size from system font
	 */
	private int m_nSystemFontSize= 0;
	/**
	 * basic system font name
	 */
	private static FontData[] m_aSystemFont= null;
	/**
	 * some fonts of system to display
	 */
	private static LinkedList<FontData[]> m_lFonts= new LinkedList<FontData[]>();
	/**
	 * current name of font
	 */
	private String m_sCurrentFontName= "";
	/**
	 * character size from current font
	 */
	private int m_nCurrentFontSize= 0;
	/**
	 * current style of font
	 */
	private int m_nCurrentFontStyle= SWT.NONE;
	/**
	 * whether font should be underlined
	 */
	private boolean underlined= false;
	/**
	 * whether in this object was
	 * an new font defined	
	 */
	private boolean m_bNewFontDefined= false;
	/**
	 * current font object for display
	 */
	private Font m_oCurrentFont= null;
	
	/**
	 * constructor to create new default object
	 * 
	 * @author Alexander Kolli
	 * @version 0.02.00, 21.02.2012
	 * @since JDK 1.6
	 */
	public FontObject()
	{
	}
	/**
	 * constructor to create an cloned object
	 * 
	 * @param otherObj make new instance with content of this other object
	 * @author Alexander Kolli
	 * @version 0.02.00, 21.02.2012
	 * @since JDK 1.6
	 */
	public FontObject(FontObject otherObj)
	{
		// do not want to make object Cloneable
		// because don't know what happen with cloned Color objects
		// whether should destroy only self generated font's and color's
		// or only the color's and font's which are created before self
		for (Color color : otherObj.m_lColors)
			m_lColors.addLast(color);
		for(String colorname : otherObj.m_lsColors)
			m_lsColors.addLast(colorname);
		m_oCurrentFont= otherObj.m_oCurrentFont;
		m_sCurrentFontName= otherObj.m_sCurrentFontName;
		m_nCurrentFontSize= otherObj.m_nCurrentFontSize;
		m_nCurrentFontStyle= otherObj.m_nCurrentFontStyle;
		m_nSystemFontSize= otherObj.m_nSystemFontSize;
		bInit= otherObj.bInit;
	}
	
	
	/**
	 * initialization of system colors.<br />
	 * <table>
	 * 	 <tr>
	 *     <td align="right">
	 *       TEXT
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_FOREGROUND 
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for text
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       TEXT_INACTIVE
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_TITLE_INACTIVE_BACKGROUND
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for inactive text
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       BACKGROUND
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_BACKGROUND
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for hole background
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       LIST_SELECTED_TEXT
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_LIST_SELECTION_TEXT
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for text when be selected inside an list
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       LIST_SELECTED_ARRAY
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_LIST_SELECTION
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color around selected text inside an list
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       LIST_BACKGROUND
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_LIST_BACKGROUND
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color of text field
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_LIGHT_SHADOW
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color of widget (button, combo, ...) in normaly state
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for border of widget
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_HOVER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_HIGHLIGHT_SHADOW
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color of widget when mouse over it
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_HOVER_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_TITLE_BACKGROUND_GRADIENT
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for border of widget when mouse over it
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_PRESSED
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_DARK_SHADOW
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color of widget when in an pressed state
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_PRESSED_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for border of widget when in an pressed state
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_INACTIVE
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_NORMAL_SHADOW
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color widget when inactive
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_INACTIVE_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_WIDGET_BORDER
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color for border of widget when inactive
	 *     </td>
	 * 	 </tr>
	 * 	 <tr>
	 *     <td align="right">
	 *       WIDGET_FOCUS
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       SWT.COLOR_TITLE_BACKGROUND
	 *     </td>
	 *     <td align="center">
	 *       -
	 *     </td>
	 *     <td align="left">
	 *       color when widget has focus
	 *     </td>
	 * 	 </tr>
	 * </table>
	 * 
	 * @param display device object to get system color
	 * @author Alexander Kolli
	 * @version 0.02.00, 21.02.2012
	 * @since JDK 1.6
	 */
	private void init(Display display)
	{
		GC gc;
		FontData[] fontData;
		String[] logicalNames;
		colors[] defColor;
		
		if(bInit)
			return;
		gc= new GC(display);
		m_nSystemFontSize= gc.getFontMetrics().getHeight();
		gc.dispose();
		
		defColor= colors.values();
		for (int i = 0; i < colors.values().length; i++)
		{
			m_lsColors.addLast(defColor[i].name());
			m_lColors.addLast(display.getSystemColor(colorIds[i]));
		}
		m_lColors.set(colors.BACKGROUND.ordinal(), HtmTags.systemColor);
		if(m_aSystemFont == null)
		{
			String sysFont;
			
			logicalNames = new String[] {	"serif", "dingbats", "sanserif", "sans serif", "DejaVu Sans", "Arial", "courier", "courier new", 
											"Tahoma", "die nasty", "Highway to Heck", "Edmunds", "Edmunds Distressed", "Hawkeye", 
											"Joystix", "Kenyan Coffee", "kimberly", "Letter Set B", "Map Of You", "Neurochrome",  
											"monospaced", "dialog", "dialoginput", "Paint Boy",
											"DejaVu Sans Mono", "Courier 10 Pitch"};
			fontData= display.getSystemFont().getFontData();
			sysFont= fontData[0].name;
			m_aSystemFont= display.getFontList(sysFont, /*scalable*/true);
			if(m_aSystemFont.length == 0)
				m_aSystemFont= display.getFontList(sysFont, /*scalable*/false);
			if(m_aSystemFont.length == 0)
				m_aSystemFont= fontData;
			for (String font : logicalNames) 
			{
				fontData= display.getFontList(font, /*scalable*/true);
				if(fontData.length == 0)
					fontData= display.getFontList(font, /*scalable*/false);
				if(	fontData.length != 0 &&
					!fontData[0].name.equals(sysFont)	)
				{
					m_lFonts.add(fontData);
					if(m_lFonts.size() >= 15)
						break;
				}
			}
			if(m_lFonts.size() < 15)
			{
				String currentFont= "";
				
				fontData= display.getFontList(null, /*scalable*/true);
				for(FontData font : fontData) 
				{
					if(	!font.name.equals(sysFont) &&
						!font.name.equals(currentFont)	)
					{
						boolean bfound= false;
						
						for (FontData[] fData : m_lFonts)
						{
							if(fData[0].name == font.name)
							{
								bfound= true;
								break;
							}
						}
						if(!bfound)
						{
							fontData= display.getFontList(font.name, /*scalable*/true);
							if(fontData.length > 0)
							{
								m_lFonts.add(fontData);
								currentFont= fontData[0].name;
							}
							if(m_lFonts.size() >= 15)
								break;
						}
					}					
				}
			}
		}
		bInit= true;
	}
	/**
	 * return basic font of system
	 * 
	 * @return system font
	 */
	public static FontData[] getSystemFont()
	{
		return m_aSystemFont;
	}
	/**
	 * return some fonts to display
	 * 
	 * @return static system fonts
	 */
	public static LinkedList<FontData[]> getotherFonts()
	{
		return m_lFonts;
	}
	/**
	 * return font size
	 * 
	 * @return size of font
	 */
	public int getSize()
	{
		if(m_oCurrentFont != null)
			return m_nCurrentFontSize;
		return m_nSystemFontSize;
	}
	/**
	 * show whether font style is bold
	 * 
	 * @return whether style is bold
	 */
	public boolean isBold()
	{
		if((m_nCurrentFontStyle & SWT.BOLD) == 0)
			return false;
		return true;
	}
	/**
	 * show whether font style is bold
	 * 
	 * @return whether style is bold
	 */
	public boolean isItalic()
	{
		if((m_nCurrentFontStyle & SWT.ITALIC) == 0)
			return false;
		return true;
	}
	/**
	 * show whether font style is underlined
	 * 
	 * @return whether style is underlined
	 */
	public boolean isUnderlined()
	{
		return underlined;
	}
	/**
	 * define new current color and also create new object when needed
	 * 
	 * @param composite device to get display for color creation
	 * @param color string of color.<br />string can be an null string when no new color needed
	 * @param type for which color type should be defined
	 * @param newObj when this parameter is set true, method create an new object when needed and give back also true, otherwise false
	 * @param fileName name of file in witch defined
	 * @return new created object or current when no changes
	 */
	public FontObject defineNewColorObj(Composite composite, String color, colors type, BooleanHolder newObj, String fileName)
	{
		FontObject newObject;

		if(color.equals(""))
		{
			newObj.value= false;
			return this;
		}
		if(newObj.value)
			newObject= new FontObject(this);
		else
			newObject= this;
		newObject.defineColor(composite, color, type, fileName);
		return newObject;
	}
	/**
	 * define current color
	 * 
	 * @param composite device to get display for color creation
	 * @param color string of color
	 * @param type for which color type should be defined
	 * @param fileName name of file in witch defined
	 */
	public void defineColor(Composite composite, String color, colors type, String fileName)
	{
		Color colorObj;
		BooleanHolder created= new BooleanHolder();
		Display display= composite.getDisplay();
		
		if(!bInit)
			init(display);
		colorObj= getColor(display, color, created, fileName);
		m_lColors.set(type.ordinal(), colorObj);
		m_lsColors.set(type.ordinal(), color);
		if(created.value)
		{
			m_aDefinedColors.add(colorObj);
/*			final Color cObj= colorObj;
			composite.addDisposeListener(new DisposeListener() 
			{				
				@Override
				public void widgetDisposed(DisposeEvent arg)
				{
					cObj.dispose();
				}
			});*/
		}
	}
	/**
	 * define color type with new color
	 * 
	 * @param color object of color
	 * @param type for which color type should be defined
	 */
	public void defineColor(Color color, colors type)
	{
		m_lColors.set(type.ordinal(), color);
	}
	/**
	 * define new current font and also create new object when needed
	 * 
	 * @param composite device to get display for font creation
	 * @param font name of font<br />string can be an null string when no new font needed
	 * @param size size of font<br />can be 0 when no new size needed
	 * @param bold whether font should be displayed as bold
	 * @param italic whether font should be displayed as italic
	 * @param underline whether font should be underlined
	 * @param newObj when this parameter is set true, method create an new object when needed and give back also true, otherwise false
	 * @return new object when defined, otherwise old
	 */
	public FontObject defineNewFontObj(Composite composite, String font, int size, boolean bold, 
			boolean italic, boolean underline, BooleanHolder newObj)
	{
		FontObject newObject;
		
		if(	font.equals("") &&
			(	size == 0 ||
				size == getSize()	) &&
			bold == isBold() &&
			italic == isItalic() &&
			underline == isUnderlined()	)
		{
			newObj.value= false;
			return this;
		}
		if(newObj.value)
			newObject= new FontObject(this);
		else
			newObject= this;
		newObject.defineFont(composite, font, size, bold, italic, underline);
		return newObject;
	}
	/**
	 * define current font
	 * 
	 * @param composite device to get display for font creation
	 * @param font name of font
	 * @param size size of font
	 * @param bold whether font should be displayed as bold
	 * @param italic whether font should be displayed as italic
	 * qparam underline whether font should be underlined
	 */
	private void defineFont(Composite composite, String font, int size, boolean bold, boolean italic, boolean underline)
	{
		int style= SWT.NONE;
		Display display= composite.getDisplay();
		Font systemFont= display.getSystemFont();
		FontData[] fontData;
		
		if(m_oCurrentFont != null)
			fontData= m_oCurrentFont.getFontData();
		else
			fontData= systemFont.getFontData();
		
		if(!bInit)
			init(display);
		style= fontData[0].getStyle();
		style= bold   ? style | SWT.BOLD   : style;
		style= italic ? style | SWT.ITALIC : style;
		underlined= underline;
		if(	fontData[0].name.equals(font.trim()) &&
			fontData[0].style == style &&
			m_nSystemFontSize == size				)
		{
			m_oCurrentFont= null;
			return;
		}
		if(size == 0)
			size= fontData[0].getHeight();
		if(font.equals(""))
			font= fontData[0].getName();
		if(	m_oCurrentFont != null &&
			m_sCurrentFontName.equals(font) &&
			m_nCurrentFontSize == size &&
			m_nCurrentFontStyle == style		)
		{
			return;
		}
		if(	m_bNewFontDefined == true &&
			m_oCurrentFont != null		)
		{
			m_oCurrentFont.dispose();
		}
		m_sCurrentFontName= font;
		m_nCurrentFontSize= size;
		m_nCurrentFontStyle= style;
		m_oCurrentFont= new Font(display, font, size, style);
		m_bNewFontDefined= true;
		
/*		final Font fObj= m_oCurrentFont;
		composite.addDisposeListener(new DisposeListener()
		{
			@Override
			public void widgetDisposed(DisposeEvent arg)
			{
				fObj.dispose();
			}
		});*/
	}
	/**
	 * set font and color inside composite
	 * 
	 * @param component widget to set font and or color object
	 */
	public void setDevice(Control component)
	{
		Display display= component.getDisplay();
		Color background, foreground, maincolor;

		if(!bInit)
			init(display);

		//return;
		// setBackgroundMode zum vererben
		// http://www.java-forum.org/awt-swing-swt/114085-bug-feature.html
		if(component instanceof Composite)
		{
			if(m_oCurrentFont != null)
				component.setFont(m_oCurrentFont);
			background= m_lColors.get(colors.BACKGROUND.ordinal());
			component.setBackground(background);
			
		}else if(component instanceof Label)
		{
			if(m_oCurrentFont != null)
				component.setFont(m_oCurrentFont);
			foreground= m_lColors.get(colors.TEXT.ordinal());
			background= m_lColors.get(colors.BACKGROUND.ordinal());
			component.setBackground(background);
			component.setForeground(foreground);
			
		}else if(component instanceof Button)
		{
			int style= ((Button)component).getStyle();

			background= m_lColors.get(colors.BACKGROUND.ordinal());
			component.setBackground(background);
			
			if(	(style & SWT.CHECK) == 0 &&
				(style & SWT.RADIO) == 0 &&
				(style & SWT.ARROW) == 0	)
			{
				if(m_oCurrentFont != null)
					component.setFont(m_oCurrentFont);			
				
				maincolor= display.getSystemColor(colorIds[colors.TEXT.ordinal()]);
				foreground= m_lColors.get(colors.TEXT.ordinal());
				if(!foreground.equals(maincolor))
				{
					boolean disposeFont= false;
					final String text;
					final Font currentFont;
					GridData data= (GridData)((Button) component).getLayoutData();
					
					text= ((Button) component).getText();
					((Button) component).setText("");
					if(	data == null ||
						data.widthHint == -1	)
					{// when no width for button be set
					 // calculate width
						GC gc= new GC(display);
	
						if(data == null)
							data= new GridData();
						gc.setFont(component.getFont());
						data.widthHint= gc.textExtent(text).x + 14;
						((Button) component).setLayoutData(data);
						gc.dispose();
					}
					if(m_oCurrentFont != null)
					{
						FontData fontData= m_oCurrentFont.getFontData()[0];
						currentFont= new Font(display, fontData.getName(), fontData.getHeight(), fontData.getStyle());
						disposeFont= true;
						
					}else
						currentFont= component.getFont();
	
		/*			component.addMouseListener(new MouseListener() {
						
						@Override
						public void mouseUp(MouseEvent arg0) {
							System.out.println("Mouse is goes up");
						}
						
						@Override
						public void mouseDown(MouseEvent arg0) {
							System.out.println("Mouse is goes down");
						}
						
						@Override
						public void mouseDoubleClick(MouseEvent arg0) {
							// TODO Auto-generated method stub
							
						}
					});*/
		/*			component.addMouseMoveListener(new MouseMoveListener() {
						
						@Override
						public void mouseMove(MouseEvent arg0) {
							System.out.println("Mouse is moving");
						}
					});*/
		/*			component.addMouseTrackListener(new MouseTrackListener() {
						
						@Override
						public void mouseHover(MouseEvent arg0) {
							System.out.println("Mouse is hover");
						}
						
						@Override
						public void mouseExit(MouseEvent arg0) {
							System.out.println("Mouse is exit");
						}
						
						@Override
						public void mouseEnter(MouseEvent arg0) {
							System.out.println("Mouse is enter");
						}
					});*/
					component.addPaintListener(new PaintListener()
					{				
						@Override
						public void paintControl(PaintEvent ev)
						{
							GC gc;
							Color foreground;
							Font font= null;
							Control control= (Control)ev.widget;
							Button button= (Button)ev.widget;
							Display display= ev.display;
							String foregroundColorName;
							BooleanHolder createdForeground= new BooleanHolder();
							int pressed= 0;
							int x = 0;
							int y = 0;
							int width;
							int height;
							int fontHeight, fontWidth;
							int ximg, yimg;
							
							if(button.isEnabled())
								foregroundColorName= m_lsColors.get(colors.TEXT.ordinal());
							else
								foregroundColorName= m_lsColors.get(colors.TEXT_INACTIVE.ordinal());
							foreground= getColor(display, foregroundColorName, createdForeground, "UNKNOWN");
							Rectangle rect= control.getBounds();
							width = rect.width;
							height = rect.height;
							
							if(button.getSelection())
								pressed= 1;
							if (width == 0)
							    width = 1;
							if (height == 0)
							    height = 1;
							//button.setImage(new Image(control.getParent().getDisplay(), width, height));
							//original = button.getImage();
							//ImageLoader loader= new ImageLoader();
							//loader.data= new ImageData[] {original.getImageData()};
							//loader.save("/home/kollia/button.jpg", SWT.IMAGE_JPEG);
							
							gc= ev.gc;// = new GC(original);
							gc.setForeground(foreground);
							if(	!m_sCurrentFontName.equals("") ||
								m_nCurrentFontSize != 0 ||
								m_nCurrentFontStyle != SWT.NONE		)
							{
								font= new Font(display, m_sCurrentFontName, m_nCurrentFontSize, m_nCurrentFontStyle);
								gc.setFont(font);
							}
							
							fontWidth= 0;
							Point metric= gc.textExtent(text);
							fontWidth= metric.x;
							fontHeight= metric.y;
		
							//ximg = (x + width) / 2 - fontSize * text.length() / 3;
							ximg= (x + width) / 2 - fontWidth / 2 + pressed;
							yimg= (y + height) / 2 - fontHeight / 2 + pressed;
							//yimg = (y + height) / 2 - fontHeight * 3 / 4;
							//System.out.println(ximg +"= ("+x+" + "+width+") / 2 - "+fontWidth+" / 2");
							
							gc.drawText(text, ximg > 4 ? ximg : 4, yimg > 4 ? yimg : 4,	SWT.DRAW_TRANSPARENT | SWT.DRAW_MNEMONIC);
							
							if(font != null)
								font.dispose();
							if(createdForeground.value)
								foreground.dispose();
						}
					});
				}
			}
			
		}else if(	component instanceof Text ||
					component instanceof Combo ||
					component instanceof List		)
		{
			if(m_oCurrentFont != null)
				component.setFont(m_oCurrentFont);
			foreground= m_lColors.get(colors.TEXT.ordinal());
			background= m_lColors.get(colors.LIST_BACKGROUND.ordinal());
			component.setBackground(background);
			//((Text)component).set
		}else
		{
			if(m_oCurrentFont != null)
				component.setFont(m_oCurrentFont);
			foreground= m_lColors.get(colors.TEXT.ordinal());
			background= m_lColors.get(colors.BACKGROUND.ordinal());
			component.setBackground(background);
		}
/*		else if(component instanceof Slider)
		{
		}else if(component instanceof Combo)
		{
		}else if(component instanceof List)
		{
		}else if(component instanceof Spinner)
		{
		}	*/
	}
	/**
	 * get current color for defined color name
	 * inside FontObject
	 * 
	 * @param color defined color name
	 * @return current color
	 */
	public Color getCurrentColor(colors color)
	{
		return m_lColors.get(color.ordinal());
	}
	/**
	 * creating color object from string defined in an layout file
	 * 
	 * @param display display object to get color object
	 * @param color string of color
	 * @return object of created color
	 * @author Alexander Kolli
	 * @version 0.02.00, 21.02.2012
	 * @since JDK 1.6
	 */
	public Color getColor(Display display, String color, BooleanHolder created, String file)
	{
		String orgStr;
		Color oRv= null;

		if(!bInit)
			init(display);
		created.value= false;
		orgStr= color;
		color= color.trim();
		if(	color.length() == 7 &&
			color.substring(0, 1).equals("#")	)
		{
			Integer red= null, green= null, blue= null;
			
			try{
				red= Integer.parseInt(color.substring(1, 3), 16);
				green= Integer.parseInt(color.substring(3, 5), 16);
				blue= Integer.parseInt(color.substring(5, 7), 16);
			}catch(NumberFormatException ex)
			{}
			if(	red != null &&
				green != null &&
				blue != null	)
			{
				oRv= new Color(display, red, green, blue);
				if(oRv != null)
					created.value= true;
			}
		}else
		{
			color= color.trim().toUpperCase();
			if(	color.length() > 4 &&
				color.substring(0, 4).equals("DARK")	)
			{
				String c= color.substring(4).trim();
				if(c.equals("GREY"))
					c= "GRAY";
				color= "DARK " + c;
				
			}else if(color.equals("GREY"))
				color= "GRAY";
			if(color.equals("BLACK"))
				oRv= display.getSystemColor(SWT.COLOR_BLACK);
			else if(color.equals("BLUE"))
				oRv= display.getSystemColor(SWT.COLOR_BLUE);
			else if(color.equals("CYAN"))
				oRv= display.getSystemColor(SWT.COLOR_CYAN);
			else if(color.equals("DARK BLUE"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_BLUE);
			else if(color.equals("DARK CYAN"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_CYAN);
			else if(color.equals("DARK GRAY"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_GRAY);
			else if(color.equals("DARK GREEN"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_GREEN);
			else if(color.equals("DARK MAGENTA"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_MAGENTA);
			else if(color.equals("DARK RED"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_RED);
			else if(color.equals("DARK YELLOW"))
				oRv= display.getSystemColor(SWT.COLOR_DARK_YELLOW);
			else if(color.equals("GRAY"))
				oRv= display.getSystemColor(SWT.COLOR_GRAY);
			else if(color.equals("GREEN"))
				oRv= display.getSystemColor(SWT.COLOR_GREEN);
			else if(color.equals("MAGENTA"))
				oRv= display.getSystemColor(SWT.COLOR_MAGENTA);
			else if(color.equals("RED"))
				oRv= display.getSystemColor(SWT.COLOR_RED);
			else if(color.equals("WHITE"))
				oRv= display.getSystemColor(SWT.COLOR_WHITE);
			else if(color.equals("YELLOW"))
				oRv= display.getSystemColor(SWT.COLOR_YELLOW);
			else
			{
				colors colorID;
				
				try{
					colorID= colors.valueOf(color);
					if(colorIds[colorID.ordinal()] == SWT.NONE)
					{
						if(color.substring(0, 20) == "WIDGET_BORDER_SHADOW")
							colorID= colors.valueOf("WIDGET_BORDER_SHADOW");
						else
							colorID= colors.valueOf("WIDGET_BORDER");
					}
				}catch(IllegalArgumentException ex)
				{
					MsgTranslator.instance().errorPool("FAULT_system_color", orgStr, file);
					return getColor(display, colors.BACKGROUND.name(), created, file);
				}
				oRv= display.getSystemColor(colorIds[colorID.ordinal()]);
			}
		}
		if(oRv == null)
		{
			if(file.substring(0, 11).equals("client.ini_"))
				MsgTranslator.instance().errorPool("FAULT_init_color", orgStr, file.substring(11));
			else
				MsgTranslator.instance().errorPool("FAULT_layout_color", orgStr, file);
		}
		return oRv;
	}
	/**
	 * dispose color and font objects
	 */
	public void dispose()
	{
		if(m_bNewFontDefined)
			m_oCurrentFont.dispose();
		for (Color obj : m_aDefinedColors)
		{
			obj.dispose();
		}
	}

}
