#include "svn_blamewidgets.h"

SvnBlameWidget::SvnBlameWidget(QWidget *parent)
    : QWidget(parent), Ui::SvnBlameWidget()
{
    Ui::SvnBlameWidget::setupUi(this);
    
    m_item = new BlameItem();
        
    m_blameModel= new BlameTreeModel(m_item);
    treeView->setModel( m_blameModel );
    treeView->setSortingEnabled(true);
    treeView->setIndentation(-7);
    treeView->sortByColumn(0, Qt::DescendingOrder);

//     m_logviewDetailedModel = new LogviewDetailedModel(m_item);
//     listView->setModel( m_logviewDetailedModel );
}

SvnBlameWidget::~SvnBlameWidget()
{}

void SvnBlameWidget::refreshWithNewData( QList<SvnBlameHolder> datalist )
{
    m_blameModel->prepareItemUpdate();
    m_item->setHolderList( datalist );
    m_blameModel->finishedItemUpdate();
    treeView->resizeColumnToContents(0);//line
    treeView->resizeColumnToContents(1);//rev
    treeView->resizeColumnToContents(2);//author
}
