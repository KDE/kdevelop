#include "svn_blamewidget.h"
#include "subversion_widget.h"
#include <qmap.h>
#include <qlistview.h>

SvnBlameWidget::SvnBlameWidget( QWidget *parent, const char* name, bool modal, WFlags f )
	:SvnBlameWidgetBase( parent, name, modal, f )
{
}
SvnBlameWidget::~SvnBlameWidget()
{}

// copy whole value. If blocking dialog box, it is expensive
// but eventually we will make this dialog box as modeless.

void SvnBlameWidget::copyBlameData( QValueList<SvnBlameHolder> *blamelist )
//void SvnBlameWidget::copyBlameData( QMap<unsigned int, SvnBlameHolder> *blamelist )
{
	m_blamelist = *blamelist;
}
	
int SvnBlameWidget::exec()
{
	outView()->clear();
	outView()->setSortColumn(0);
// 	QMap<unsigned int, SvnBlameHolder>::Iterator it;
	QValueList<SvnBlameHolder>::Iterator it;
	
	for( it = m_blamelist.begin(); it != m_blamelist.end(); ++it ){
		
		SvnBlameHolder holder = *it;
		SvnIntSortListItem *item = new SvnIntSortListItem(outView());
		
		QString prettyDate = holder.date.left(16).replace(10, 1, ' ');
		
		item->setText(0, QString::number( holder.line+1 ) ); 
		item->setText(1, QString::number(holder.rev) );
		item->setText(2, prettyDate );
		item->setText(3, holder.author );
		item->setText(4, holder.content );
		
	}
	outView()->sort();
	return SvnBlameWidgetBase::exec();
}

QListView* SvnBlameWidget::outView()
{
	return listView1;
}
// TODO eventually, to implement some interactive functions with IDE, this widget should be modeless
// void SvnBlameWidget::show()

#include "svn_blamewidget.moc"
