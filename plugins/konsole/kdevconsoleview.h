/***************************************************************************
*   Copyright (C) 2003 by KDevelop Authors                                *
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
#include <QString>
#include <QVBoxLayout>

class KDevProject;
class KDevKonsoleViewPart;

namespace KParts
{
class ReadOnlyPart;
class Part;
}
class QVBoxLayout;


class KDevKonsoleView : public QWidget
{
    Q_OBJECT
public:
    KDevKonsoleView( KonsoleViewPart *part );
    virtual ~KDevKonsoleView();

public slots:
    void setDirectory( const KUrl &dirUrl );

private slots:
    void activePartChanged( KParts::Part *activatedPart );
    void partDestroyed();

protected:
    virtual void showEvent( QShowEvent *event );

private:
    void activate();

    KDevDocumentViewPart *m_part;
    QVBoxLayout *m_vbox;
};


#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
