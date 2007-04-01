#ifndef SVN_LOGVIEWWIDGET
#define SVN_LOGVIEWWIDGET
#include "ui_svnlogviewwidget.h"
#include "svn_models.h"
#include "subversion_part.h"
#include "subversion_core.h"

class QModelIndex;

class SvnLogviewWidget : public QWidget, public Ui::SvnLogviewWidget
{
    Q_OBJECT
public:
    SvnLogviewWidget( KUrl &url, KDevSubversionPart *part, QWidget *parent );
    virtual ~SvnLogviewWidget();
    void refreshWithNewData( QList<SvnLogHolder> datalist );
protected:
    virtual void contextMenuEvent( QContextMenuEvent * event );
    
protected slots:
    void customContextMenuEvent( const QPoint &point );
    void blameRev();
    void diffToPrev();
    void treeViewClicked( const QModelIndex &index );
    void listViewClicked( const QModelIndex &index );

private:
    KDevSubversionPart *m_part;
    LogItem *m_item;
    LogviewTreeModel *m_logviewModel;
    LogviewDetailedModel *m_logviewDetailedModel;

    QModelIndex m_contextIndex;
    KUrl m_url;
};

#endif
