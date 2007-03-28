#include "svn_blamewidget.h"
#include "subversion_widget.h"
#include <qmap.h>
#include <qlistview.h>
#include <qlayout.h>
#include <klocale.h>

SvnBlameWidget::SvnBlameWidget( QWidget *parent, const char* name, bool modal, WFlags f )
	:QWidget( parent )
{
    m_layout = new QVBoxLayout( this, 1, 1 );
    m_layout->setMargin(1);
    
    m_listView = new QListView( this );
    outView()->setAllColumnsShowFocus( TRUE );
    outView()->addColumn( i18n("Line") );
    outView()->addColumn( i18n("Rev") );
    outView()->addColumn( i18n("Date") );
    outView()->addColumn( i18n("Author") );
    outView()->addColumn( i18n("Content") );
    
    m_layout->addWidget( m_listView );
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
	
void SvnBlameWidget::show()
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
	QWidget::show();
}

QListView* SvnBlameWidget::outView()
{
    return m_listView;
}
// TODO eventually, to implement some interactive functions with IDE, this widget should be modeless
// void SvnBlameWidget::show()

#include "svn_blamewidget.moc"
