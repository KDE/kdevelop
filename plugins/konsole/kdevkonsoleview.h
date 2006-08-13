/***************************************************************************
*   Copyright (C) 2003, 2006 by KDevelop Authors                          *
*   kdevelop-devel@kde.org                                                *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVKONSOLEVIEW_H
#define KDEVKONSOLEVIEW_H

#include <QWidget>

#include <kurl.h>

class QVBoxLayout;

class KDevDocument;

namespace KParts
{
class ReadOnlyPart;
}

class KDevKonsoleView : public QWidget
{
    Q_OBJECT
public:
    KDevKonsoleView( QWidget *parent = 0 );
    virtual ~KDevKonsoleView();

public slots:
    void setDirectory( const KUrl &dirUrl );

private slots:
    void projectOpened();
    void projectClosed();
    void documentActivated( KDevDocument *document );
    void partDestroyed();

private:
    void init();

    KParts::ReadOnlyPart *m_part;
    QVBoxLayout *m_vbox;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
