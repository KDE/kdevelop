#ifndef _OUTPUTVIEWS_H_
#define _OUTPUTVIEWS_H_

#include "kdevcomponent.h"


class MakeWidget;
class CProject;

class MakeView : public KDevComponent
{
    Q_OBJECT

public:
    MakeView( QObject *parent=0, const char *name=0 );
    ~MakeView();

protected:
    virtual void setupGUI();
    virtual void commandRequested(const QString &command);
    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
    
private:
    MakeWidget *m_widget;
    friend class MakeWidget;

    CProject *m_prj;
};


class AppOutputWidget;

class AppOutputView : public KDevComponent
{
    Q_OBJECT

public:
    AppOutputView( QObject *parent=0, const char *name=0 );
    ~AppOutputView();

protected:
    virtual void setupGUI();
    virtual void stopButtonClicked();
    
private:
    AppOutputWidget *m_widget;
    friend class AppOutputWidget;
};
#endif
