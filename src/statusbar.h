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

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
namespace KTextEditor { class ViewStatusMsgInterface; }
# endif
#endif

namespace KTextEditor { class ViewCursorInterface; }
namespace KParts { class Part; }

class StatusBar : public KStatusBar
{
    Q_OBJECT

public:
    StatusBar( QWidget *parent=0, const char *name=0 );
    ~StatusBar();
    void addWidget ( QWidget *widget, int stretch = 0, bool permanent = FALSE );

private slots:
    void cursorPositionChanged();
    void activePartChanged(KParts::Part *part);
    void setStatus(const QString &str);
    void setCursorPosition(int line, int col);

private:
    QLabel *_status;

	KTextEditor::ViewCursorInterface * _cursorIface;
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
	KTextEditor::ViewStatusMsgInterface * _viewmsgIface;
# endif
#endif
  KParts::Part *_activePart;

// still hoping for a fix for KDE-3.1
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
	QMap<KParts::Part*, QString> _map;
# endif
#endif

};

#endif
