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

#ifndef _STATUSBAR_H_
#define _STATUSBAR_H_

#include <kstatusbar.h>
#include <qmap.h>

class QLabel;

namespace KTextEditor { class View; }
namespace KParts { class Part; }

/**
Status bar.
*/
class KDevStatusBar : public KStatusBar
{
    Q_OBJECT

public:
    KDevStatusBar( QWidget *parent=0, const char *name=0 );
    ~KDevStatusBar();
    void addWidget ( QWidget *widget, int stretch = 0, bool permanent = FALSE );

private slots:
    void statusChanged();
    void activePartChanged(KParts::Part *part);

private:
    QLabel *_status;

    KParts::Part *_activePart;
    KTextEditor::View *_view;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
