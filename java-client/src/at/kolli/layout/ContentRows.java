package at.kolli.layout;

import java.security.acl.Group;
import java.util.ArrayList;

import org.eclipse.swt.widgets.Composite;

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
	public int getMaxColumns()
	{
		int columns= 0;
		
		for(HtmTags tag : m_lContent)
		{
			ContentFields field= (ContentFields)tag;
			
			columns+= field.colspan;
		}
		return columns;
	}
	
	/**
	 * Overridden method of execute
	 * which start the execute in fields class (td) to calibrate
	 * on screen (in window)
	 * 
	 * @param composite parent {@link Composite}
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite)
	{
		execute(composite, null);
	}
	
	/**
	 * method to generate the widget in the display window
	 * 
	 * @param composite parent {@link Composite}
	 * @param isAlsoNextColumn ArrayList to see how much rows the field in the row before needed
	 * @author Alexander Kolli
	 * @version 1.00.00, 07.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, ArrayList<Integer> isAlsoNextColumn)
	{
		int column= 0;
		int maxColumns= isAlsoNextColumn.size();
		
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
			tag.execute(composite);
		}
		while(column < maxColumns)
		{
			ContentFields field= new ContentFields();
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
			
			if(column < maxColumns)
			{
				insert(field);
				field.setBorder(m_bBorder);
				field.execute(composite);
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
