/*
   Copyright (C) 2002 by Roberto Raggi <raggi@cli.di.unipi.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef PROBLEMSREPORTER_H
#define PROBLEMSREPORTER_H

#include <qlistview.h>

class CppSupportPart;
class QTimer;

namespace KParts{
    class Part;
}

namespace KTextEditor{
    class EditInterface;
    class Document;
}

class ProblemReporter: public QListView{
    Q_OBJECT
public:
    ProblemReporter( CppSupportPart* part, QWidget* parent=0, const char* name=0 );
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

private slots:
    void slotActivePartChanged( KParts::Part* );
    void slotTextChanged();
    void slotSelected( QListViewItem* );

private:
    CppSupportPart* m_cppSupport;
    KTextEditor::EditInterface* m_editor;
    KTextEditor::Document* m_document;
    QTimer* m_timer;
    QString m_filename;
    int m_active;
    int m_delay;
};

#endif
