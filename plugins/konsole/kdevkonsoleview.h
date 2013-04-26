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

#ifndef KDEVPLATFORM_PLUGIN_KDEVKONSOLEVIEW_H
#define KDEVPLATFORM_PLUGIN_KDEVKONSOLEVIEW_H

#include <QWidget>

class KUrl;


class KDevKonsoleViewPlugin;


class KDevKonsoleView : public QWidget
{
    Q_OBJECT
public:
    explicit KDevKonsoleView( KDevKonsoleViewPlugin* plugin, QWidget *parent = 0 );
    virtual ~KDevKonsoleView();

public slots:
    void setDirectory( const KUrl &dirUrl );

protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );

private :
    class KDevKonsoleViewPrivate* const d;

    Q_PRIVATE_SLOT( d, void _k_slotTerminalClosed() )
};

#endif

