/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TEXTTOOLSWIDGET_H_
#define _TEXTTOOLSWIDGET_H_

#include <klistview.h>
#include <qstringlist.h>

class TextToolsPart;
class QTimer;
namespace KParts { class Part; }
namespace KTextEditor { class EditInterface; }


class TextToolsWidget : public KListView
{
    Q_OBJECT

public:
    enum Mode { HTML, Docbook, LaTeX };
    
    TextToolsWidget( TextToolsPart *part, QWidget *parent=0, const char *name=0 );
    ~TextToolsWidget();

    void setMode(Mode mode, KParts::Part *part);
    void stop();

private slots:
    void slotItemPressed(int button, Q3ListViewItem *item);
    void slotReturnPressed(Q3ListViewItem *item);
    void slotContextMenu(KListView *, Q3ListViewItem *item, const QPoint &p);

    void startTimer();
    void parseXML();
    void parseLaTeX();

private:
    TextToolsPart *m_part;
    QTimer *m_timer;
    QString m_cachedText;
    KTextEditor::EditInterface *m_editIface;
    QStringList m_relevantTags, m_emptyTags;
};

#endif
