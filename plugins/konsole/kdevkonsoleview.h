/***************************************************************************
*   Copyright 2003, 2006 by KDevelop Authors                          *
*   kdevelop-devel@kde.org                                                *
*   Copyright 2007 by Andreas Pakulat <apaku@gmx.de>                  *
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

class KUrl;


class KDevKonsoleViewPart;


class KDevKonsoleView : public QWidget
{
    Q_OBJECT
public:
    explicit KDevKonsoleView( KDevKonsoleViewPart* part, QWidget *parent = 0 );
    virtual ~KDevKonsoleView();

public slots:
    void setDirectory( const KUrl &dirUrl );

private :
    struct KDevKonsoleViewPrivate* const d;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
