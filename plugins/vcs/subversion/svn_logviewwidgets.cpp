#include "svn_logviewwidgets.h"
#include "svn_models.h"

#include <kaction.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QContextMenuEvent>
#include <QCursor>

SvnLogviewWidget::SvnLogviewWidget( KUrl &url, KDevSubversionPart *part, QWidget *parent )
    :QWidget(parent), Ui::SvnLogviewWidget()
    ,m_url(url), m_part(part)
{
    Ui::SvnLogviewWidget::setupUi(this);
    
    m_item = new LogItem();
    m_logviewModel= new LogviewTreeModel(m_item);
    treeView->setModel( m_logviewModel );
    treeView->sortByColumn(0, Qt::DescendingOrder);
    treeView->setContextMenuPolicy( Qt::CustomContextMenu );

    m_logviewDetailedModel = new LogviewDetailedModel(m_item);
    listView->setModel( m_logviewDetailedModel );

    connect( treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(treeViewClicked(const QModelIndex &)) );
    connect( treeView, SIGNAL(customContextMenuRequested( const QPoint & )),
             this, SLOT( customContextMenuEvent( const QPoint & ) ) );
    connect( listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(listViewClicked(const QModelIndex &)) );
}
SvnLogviewWidget::~SvnLogviewWidget()
{}
void SvnLogviewWidget::refreshWithNewData( QList<SvnLogHolder> datalist )
{
    m_logviewModel->prepareItemUpdate();
    m_item->setHolderList( datalist );
    m_logviewModel->finishedItemUpdate();
    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
}

void SvnLogviewWidget::customContextMenuEvent( const QPoint &point )
{
    m_contextIndex = treeView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        kDebug() << " contextMenu is not in TreeView " << endl;
        return;
    }

    KMenu menu( this );

    QAction *action = menu.addAction(i18n("Blame this Revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );
    menu.exec( treeView->viewport()->mapToGlobal(point) );
}

void SvnLogviewWidget::contextMenuEvent( QContextMenuEvent * event )
{
    kDebug() << " SvnLogviewWidget::contextMenuEvent " << endl;
//     m_contextIndex = treeView->indexAt( event->pos() );
//     if( !m_contextIndex.isValid() ){
//         kDebug() << " contextMenu is not in TreeView " << endl;
//         return;
//     }
// 
//     KMenu menu( this );
// 
//     QAction *action = menu.addAction(i18n("Blame this Revision"));
//     connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );
//     menu.exec(event->globalPos());
//
    QWidget::contextMenuEvent( event );
}

void SvnLogviewWidget::blameRev()
{
    long rev = m_logviewModel->revision( m_contextIndex );
    if( rev == -1 ){ //error
        return;
    }
    KUrl::List list;
    list << m_url;
    m_part->svncore()->spawnBlameThread(m_url, true,  0, "", rev, "" );
}
void SvnLogviewWidget::treeViewClicked( const QModelIndex &index )
{
    m_logviewDetailedModel->setNewRevision( index );
    
//     QMenu menu( this );
//     QAction *action = menu.addAction( i18n( "Blame this revision" ) );
//     connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );
//     menu.exec(QCursor::pos());
}
void SvnLogviewWidget::listViewClicked( const QModelIndex &index )
{
}
// void SvnLogviewWidget::printLog(SubversionJob *j)
// {
// 
// }
#include "svn_logviewwidgets.moc"
