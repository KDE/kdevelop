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

class QLabel;
namespace KTextEditor { class ViewCursorInterface; };
namespace KParts { class Part; };


class StatusBar : public KStatusBar
{
    Q_OBJECT
    
public:
    StatusBar( QWidget *parent=0, const char *name=0 );
    ~StatusBar();
    void addWidget ( QWidget *widget, int stretch = 0, bool permanent = FALSE );

public slots:
    void setEditorStatusVisible(bool visible);
    void setStatus(const QString &str);
    void setCursorPosition(int line, int col);
    void setModified(bool isModified);

private slots:
    void cursorPositionChanged();
    void activePartChanged(KParts::Part *part);

private:
    QLabel *_modified;
    QLabel *_cursorPosition;
    QLabel *_status;

    KTextEditor::ViewCursorInterface *m_cursorIface;
    KParts::Part *m_activePart;

};

#endif
