#ifndef SVN_BLAMEWIDGET_H
#define SVN_BLAMEWIDGET_H

#include "ui_svnblamewidget.h"
#include "svn_models.h"

class SvnBlameWidget : public QWidget, public Ui::SvnBlameWidget{
public:
    SvnBlameWidget( QWidget *parent );
    virtual ~SvnBlameWidget();
    void refreshWithNewData( QList<SvnBlameHolder> datalist );
private:
    BlameItem *m_item;
    BlameTreeModel *m_blameModel;
};

#endif
