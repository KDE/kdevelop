#ifndef _GREPVIEW_H_
#define _GREPVIEW_H_

#include "kdevcomponent.h"


class GrepWidget;

class GrepView : public KDevComponent
{
    Q_OBJECT

public:
    GrepView( QObject *parent=0, const char *name=0 );
    ~GrepView();

protected:
    virtual void setupGUI();
    virtual void createConfigWidget(KDialogBase *dlg);
    virtual void compilationAborted();
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
    
private:
    GrepWidget *m_widget;
    friend class GrepWidget;
};

#endif
