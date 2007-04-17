#ifndef SVNKJOB_H
#define SVNKJOB_H

#include <kjob.h>

class SubversionThread;

class SvnKJobBase : public KJob
{
    Q_OBJECT
public:
    friend class SubversionThread;
    
    SvnKJobBase( int type, QObject *parent );
    virtual ~SvnKJobBase();
    
    void setSvnThread( SubversionThread *job );
    SubversionThread *svnThread();
    int type();
    QString smartError();

    virtual void start();
//     SvnUiDelegate* ui();

// public Q_SLOTS:
//     bool requestKill();
    
protected Q_SLOTS:
    void threadFinished();

protected:
    // The forceful termination of thread causes deadlock in some cases.
    // Currently not used.
    // TODO 
    virtual bool doKill();

protected:
    class Private;
    Private *const d;
};

#endif
