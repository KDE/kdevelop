#ifndef _DOCTREEVIEW_H_
#define _DOCTREEVIEW_H_

#include "kdevcomponent.h"


class DocTreeWidget;

class DocTreeView : public KDevComponent
{
    Q_OBJECT

public:
    DocTreeView( QObject *parent=0, const char *name=0 );
    ~DocTreeView();

    // This is currently called by DocTreeViewConfigWidget if the
    // changes are accepted. Maybe this could be done through
    // the component system (but maybe not ;-)
    void configurationChanged() {};

protected:
    virtual void setupGUI();
    virtual void configWidgetRequested(KDialogBase *dlg);
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
    
signals:
    void projectAPISelected();
    void projectManualSelected();

private:
    DocTreeWidget *m_widget;
    friend class DocTreeWidget;
};

#endif
