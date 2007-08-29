/***************************************************************************
*   Copyright 2003, 2006 Adam Treat <treat@kde.org>                       *
*   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVKONSOLEVIEW_PART_H
#define KDEVKONSOLEVIEW_PART_H

#include "iplugin.h"
#include <QtCore/QVariant>

class KDevKonsoleViewFactory;
class KPluginFactory;

class KDevKonsoleViewPart : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    KDevKonsoleViewPart( KPluginFactory*, QObject *parent, const QVariantList & = QVariantList() );
    virtual ~KDevKonsoleViewPart();
    KPluginFactory* konsoleFactory() const;
private:
    class KDevKonsoleViewFactory *m_factory;
    KPluginFactory* m_konsoleFactory;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
