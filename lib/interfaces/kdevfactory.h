/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVFACTORY_H_
#define _KDEVFACTORY_H_

#include <klibloader.h>

class KDevApi;
class KDevPart;


class KDevFactory : public KLibFactory
{
    Q_OBJECT
    
public:
    KDevFactory( QObject *parent=0, const char *name=0 );
    ~KDevFactory();

    /**
     * Creates a KDevelop part. Normally used only by the application core.
     */
    KDevPart *createPart(KDevApi *api, QObject *parent, const QStringList &args=QStringList());

protected:
    /**
     * This should be reimplemented by any part and create.
     * Note that the implementation does not have to take care
     * of reference counting.
     */
    virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args) = 0;

private:
    virtual QObject *createObject( QObject *, const char *,
                                   const char *, const QStringList & );
};

#endif
