#ifndef KDEVMUTEX_H
#define KDEVMUTEX_H

#include "qglobal.h"

#if QT_VERSION < 0x030100

#include <qmutex.h>

class QMutexLocker
{
public:
    QMutexLocker( QMutex * );
    ~QMutexLocker();

    QMutex *mutex() const;

private:
    QMutex *mtx;

#if defined(Q_DISABLE_COPY)
    QMutexLocker( const QMutexLocker & );
    QMutexLocker &operator=( const QMutexLocker & );
#endif
};

inline QMutexLocker::QMutexLocker( QMutex *m )
    : mtx( m )
{
    if ( mtx ) mtx->lock();
}

inline QMutexLocker::~QMutexLocker()
{
    if ( mtx ) mtx->unlock();
}

inline QMutex *QMutexLocker::mutex() const
{
    return mtx;
}

#endif // QT_VERSION

#endif
