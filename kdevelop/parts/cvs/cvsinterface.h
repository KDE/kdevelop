#ifndef _CVSINTERFACE_H_
#define _CVSINTERFACE_H_

#include "kdevversioncontrol.h"


class CvsInterface : public KDevVersionControl
{
    Q_OBJECT
    
public:
    CvsInterface( QObject *parent=0, const char *name=0 );
    ~CvsInterface();

protected:
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);

    virtual void add(const QString &filename);
    virtual void remove(const QString &filename);
    virtual void update(const QString &filename);
    virtual void commit(const QString &filename);
    virtual KDevVersionControl::State registeredState(const QString &filename);

private:
    QString m_projectpath;
};

#endif
