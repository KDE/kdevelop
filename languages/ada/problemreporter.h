/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Oliver Kellogg <okellogg@users.sourceforge.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PROBLEMSREPORTER_H
#define PROBLEMSREPORTER_H

#include <qlistview.h>

class AdaSupportPart;
class QTimer;
class KDialogBase;
class BackgroundParser;

namespace KParts{
    class Part;
}

namespace KTextEditor{
    class EditInterface;
	class MarkInterface;
    class Document;
}

class ProblemReporter: public QListView{
    Q_OBJECT
public:
    ProblemReporter( AdaSupportPart* part, QWidget* parent=0, const char* name=0 );
    virtual ~ProblemReporter();

    virtual void reportError( QString message, QString filename,
                              int line, int column );

    virtual void reportWarning( QString message, QString filename,
                                int line, int column );

    virtual void reportMessage( QString message, QString filename,
                                int line, int column );

public slots:
    void reparse();
    void configure();
    void configWidget( KDialogBase* );

private slots:
    void slotPartAdded( KParts::Part* );
    void slotPartRemoved( KParts::Part* );
    void slotActivePartChanged( KParts::Part* );
    void slotTextChanged();
    void slotSelected( QListViewItem* );

private:
    AdaSupportPart* m_adaSupport;
    KTextEditor::EditInterface* m_editor;
    KTextEditor::Document* m_document;
    KTextEditor::MarkInterface* m_markIface;
    QTimer* m_timer;
    QString m_filename;
    int m_active;
    int m_delay;
    BackgroundParser* m_bgParser;
};

#endif
