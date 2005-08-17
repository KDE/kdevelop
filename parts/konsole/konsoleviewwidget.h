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

#ifndef __KONSOLEVIEWWIDGET_H__
#define __KONSOLEVIEWWIDGET_H__


#include <qwidget.h>
#include <qstring.h>
//Added by qt3to4:
#include <QVBoxLayout>


class KDevProject;
class KonsoleWidgetPrivate;
class KonsoleViewPart;
namespace KParts {
  class ReadOnlyPart;
  class Part;
}
class QVBoxLayout;


class KonsoleViewWidget : public QWidget
{
    Q_OBJECT
public:
    KonsoleViewWidget(KonsoleViewPart *part);
    virtual ~KonsoleViewWidget();

    virtual void show();

public slots:
    void setDirectory(const KURL &dirUrl);

private slots:
    void activePartChanged(KParts::Part *activatedPart);
    void partDestroyed();

private:
    void activate();

    KParts::ReadOnlyPart *part;
    QVBoxLayout *vbox;
    KonsoleViewPart *owner;
};


#endif
