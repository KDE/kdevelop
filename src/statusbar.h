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

namespace KTextEditor { class ViewStatusMsgInterface; }
namespace KTextEditor { class ViewCursorInterface; }
namespace KParts { class Part; }

class KDevStatusBar : public KStatusBar
{
    Q_OBJECT

public:
    KDevStatusBar( QWidget *parent=0, const char *name=0 );
    ~KDevStatusBar();
    void addWidget ( QWidget *widget, int stretch = 0, bool permanent = FALSE );

private slots:
    void cursorPositionChanged();
    void activePartChanged(KParts::Part *part);
    void setStatus(const QString &str);
    void setCursorPosition(int line, int col);

private:
    QLabel *_status;

	KTextEditor::ViewCursorInterface * _cursorIface;
	KTextEditor::ViewStatusMsgInterface * _viewmsgIface;
	KParts::Part *_activePart;

};

#endif
