#ifndef _OUTPUTVIEWS_H_
#define _OUTPUTVIEWS_H_

#include "kdevcomponent.h"


class MakeWidget;
class CProject;

class MakeView : public KDevComponent
{
    Q_OBJECT

public:
    MakeView( QWidget *parent=0, const char *name=0 );
    ~MakeView();

protected:
    virtual void compilationStarted(const QString &command);
    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
    
private:
    MakeWidget *makeWidget()
        { return (MakeWidget*) KDevComponent::widget(); }
    friend class MakeWidget;

    CProject *m_prj;
};


class AppOutputWidget;

class AppOutputView : public KDevComponent
{
    Q_OBJECT

public:
    AppOutputView( QWidget *parent=0, const char *name=0 );
    ~AppOutputView();

protected:
    virtual void compilationAborted();
    
private:
    AppOutputWidget *appOutputWidget()
        { return (AppOutputWidget*) KDevComponent::widget(); }
    friend class AppOutputWidget;
};
#endif
