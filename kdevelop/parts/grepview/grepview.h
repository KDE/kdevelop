#ifndef _GREPVIEW_H_
#define _GREPVIEW_H_

#include "kdevcomponent.h"


class GrepWidget;

class GrepView : public KDevComponent
{
    Q_OBJECT

public:
    GrepView( QWidget *parent=0, const char *name=0 );
    ~GrepView();

protected:
    virtual void compilationAborted();
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);
    
signals:
    void itemSelected(const QString &filename, int linenumber);
    
private:
    GrepWidget *grepWidget()
        { return (GrepWidget*) KDevComponent::widget(); }
    friend class GrepWidget;
};

#endif
