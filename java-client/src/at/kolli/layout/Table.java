package at.kolli.layout;

import java.util.ArrayList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;

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
	 * actual field (column) in the row
	 */
	protected ContentFields m_oAktField= null;
	/**
	 * whether the user want to see an border by all composites<br />
	 * in this case it will be created groups
	 */
	private boolean m_bBorder= false;
	
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
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	protected void insert(HtmTags newTag)
	{
		HtmTags list;
		ContentRows row;
		
		if(!(newTag instanceof ContentFields))
		{
			if(m_oAktField == null)
			{
				System.out.println("no correct table handling.");
				System.out.println("tag table must have follow content:");
				System.out.println("                 <table>");
				System.out.println("                    <tr>");
				System.out.println("                       <td>");
				System.out.println("                         <component> , other tables or text");
				System.out.println("                       </td>");
				System.out.println("                    </tr>");
				System.out.println("                 </table>");
				System.out.println();
				System.exit(1);
			}
			newTag.m_oParent= this;
			m_oAktField.insert(newTag);
			return;
		}
		list= m_lContent.get(m_nContent);		
		list.insert(newTag);
		if(newTag instanceof ContentFields)
		{
			m_oAktField= (ContentFields)newTag;
			m_oAktField.setBorder(m_bBorder);
			m_oAktField.align= align;
			m_oAktField.valign= valign;
		}else
		{
			row= (ContentRows)newTag;
			row.setBorder(m_bBorder);
			row.align= align;
			row.valign= valign;
		}
	}

	/**
	 * method create an new row
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void nextLine()
	{
		ContentRows cr= new ContentRows();
		
		++m_nContent;
		cr.setBorder(m_bBorder);
		cr.cellpadding= this.cellpadding;
		m_lContent.add(cr);
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
		int columns= 1;
		GridLayout layout= new GridLayout();
		ArrayList<Integer> isAlsoNext= new ArrayList<Integer>();
		
		if(actPermission.equals(permission.None))
			return;
		for(HtmTags tag : m_lContent)
		{
			ContentRows row= (ContentRows)tag;
			int c= row.getMaxColumns();
			
			if(c > columns)
				columns= c;
		}
		for(int o= 0; o<columns; ++o)
			isAlsoNext.add(0);
		layout.numColumns= columns;
		layout.marginWidth= cellspacing;
		layout.marginHeight= cellspacing;
		layout.horizontalSpacing= cellspacing;
		layout.verticalSpacing= cellspacing;
		
		if(m_bBorder)
			m_oComposite= new Group(composite, SWT.SHADOW_NONE);
		else
			m_oComposite= new Composite(composite, SWT.NONE);
		m_oComposite.setLayout(layout);
		for(HtmTags tag : m_lContent)
		{
			ContentRows row= (ContentRows)tag;
			
			row.execute(m_oComposite, isAlsoNext);
		}
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
		if(set < 1)
			m_bBorder= false;
		else
			m_bBorder= true;
	}
	
	/**
	 * whether a border should be set or not
	 * 
	 * @param set boolean value whether an border should be display
	 * @serial
	 * @see
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void setBorder(boolean set)
	{
		m_bBorder= set;
	}
}
