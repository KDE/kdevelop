/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

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

#include <klistview.h>
#include <qguardedptr.h>

class JavaSupportPart;
class QTimer;
class KDialogBase;
class Problem;

namespace KParts{
    class Part;
}

namespace KTextEditor{
    class MarkInterface;
    class Document;
}

class ProblemReporter: public KListView{
    Q_OBJECT
public:
    ProblemReporter( JavaSupportPart* part, QWidget* parent=0, const char* name=0 );
    virtual ~ProblemReporter();

    void removeAllProblems( const QString& filename );
    void reportProblem( const QString& fileName, const Problem& p );

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
    QString levelToString( int level ) const;
    int levelToMarkType( int level ) const;

private:
    JavaSupportPart* m_javaSupport;
    QGuardedPtr<KTextEditor::Document> m_document;
    KTextEditor::MarkInterface* m_markIface;
    QTimer* m_timer;
    QString m_fileName;
    int m_active;
    int m_delay;
};

#endif
