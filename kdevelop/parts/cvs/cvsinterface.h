/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    virtual void projectClosed() {};
    virtual void projectOpened(CProject *prj) {};

    virtual void addToRepositoryRequested(const QString &fileName);
    virtual void removeFromRepositoryRequested(const QString &fileName);
    virtual void updateFromRepositoryRequested(const QString &fileName);
    virtual void commitToRepositoryRequested(const QString &fileName);
    // virtual KDevVersionControl::State registeredState(const QString &fileName);

private:
    QString m_projectpath;
};

#endif
