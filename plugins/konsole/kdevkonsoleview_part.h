/***************************************************************************
*   Copyright (C) 2003 by KDevelop Authors                                *
*   kdevelop-devel@kde.org                                                *
*   Copyright (C) 2007 by Andreas Pakulat <apaku@gmx.de>                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef __KDEVPART_KDEVKONSOLEVIEW_H__
#define __KDEVPART_KDEVKONSOLEVIEW_H__

#include "iplugin.h"

class KDevKonsoleViewFactory;

class KDevKonsoleViewPart : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    KDevKonsoleViewPart( QObject *parent, const QStringList & );
    virtual ~KDevKonsoleViewPart();

private:
    class KDevKonsoleViewFactory *m_factory;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
