#ifndef _DOCTREEVIEW_H_
#define _DOCTREEVIEW_H_

#include "kdevcomponent.h"
#include "doctreewidget.h"


class DocTreeView : public KDevComponent
{
    Q_OBJECT

public:
    DocTreeView( QWidget *parent=0, const char *name=0 );
    ~DocTreeView();

    // This is currently called by DocTreeViewConfigWidget if the
    // changes are accepted. Maybe this could be done through
    // the component system (but maybe not ;-)
    void configurationChanged();

protected:
    virtual void docPathChanged();
//    virtual void createConfigWidget(CustomizeDialog *parent);
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
    virtual QWidget *widget();
    
signals:
    void fileSelected(const QString &url_file);
    void projectAPISelected();
    void projectManualSelected();

private:
    DocTreeWidget *m_widget;
    friend class DocTreeWidget;
};

#endif
