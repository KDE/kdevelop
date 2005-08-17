/* 
   Copyright (C) 2003 ian reinhart geiser <geiseri@kde.org> 

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kjsproblems.h"
#include "kjssupport_part.h"
#include <kiconloader.h>

class KJSProblemItem: public KListViewItem
{
	public:
		KJSProblemItem( Q3ListView* parent, const QString& level, const QString& problem,
		             const QString& file, const QString& line, const QString& column  )
				: KListViewItem( parent, level, problem, file, line, column )
		{}

		KJSProblemItem( Q3ListViewItem* parent, const QString& level, const QString& problem,
		             const QString& file, const QString& line, const QString& column  )
				: KListViewItem( parent, level, problem, file, line, column )
		{}

		int compare( Q3ListViewItem* item, int column, bool ascending ) const
		{
			if( column == 2 || column == 3 )
			{
				int a = text( column ).toInt();
				int b = item->text( column ).toInt();
				if( a == b )
					return 0;
				return( a > b ? 1 : -1 );
			}
			return KListViewItem::compare( item, column, ascending );
		}

};

KJSProblems::KJSProblems(kjsSupportPart *part, QWidget *parent, const char *name) : KListView(parent,name), m_part(part)
{
	addColumn ("File");
	addColumn ("Line #");
	addColumn ("Problem:");
	setIcon( SmallIcon("info") );
}


KJSProblems::~KJSProblems()
{

}

void KJSProblems::clearItems()
{
	clear();
	setIcon( SmallIcon("info") );
}

void KJSProblems::addLine(const QString &file, int lineNo, const QString &message)
{
	new Q3ListViewItem( this,  file, QString::number( lineNo ), message);
	setIcon( SmallIcon("error") );
}


#include "kjsproblems.moc"
