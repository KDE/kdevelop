#ifndef _MDIMAINFRMCOMPONENT_H_
#define _MDIMAINFRMCOMPONENT_H_

#include "kdevcomponent.h"


class MdiFrame;

class MdiMainFrmComponent : public KDevComponent
{
    Q_OBJECT

public:
    MdiMainFrmComponent( QObject *parent=0, const char *name=0 );
    ~MdiMainFrmComponent();

    // This is currently called by MdiMainFrmComponentConfigWidget if the
    // changes are accepted. Maybe this could be done through
    // the component system (but maybe not ;-)
//    void configurationChanged();

protected:
    virtual void setupGUI();
//    virtual void createConfigWidget(CustomizeDialog *parent);
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
    
private:
    MdiFrame *m_widget;
    friend class MdiFrame;
};

#endif
