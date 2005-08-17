/***************************************************************************
 *   Copyright (C) 2002 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ABBREVPART_H_
#define _ABBREVPART_H_

#include <qmap.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3ValueList>
#include "kdevplugin.h"

#include <ktexteditor/codecompletioninterface.h>

class KDialogBase;

namespace KParts{
    class Part;
}

namespace KTextEditor{
    class Document;
    class EditInterface;
    class ViewCursorInterface;
}

struct CodeTemplate {
    QString name;
    QString description;
    QString code;
    QString suffixes;
};

class CodeTemplateList {
public:
    CodeTemplateList();
    ~CodeTemplateList();

    QMap<QString, CodeTemplate* > operator[](QString suffix);
    void insert(QString name, QString description, QString code, QString suffixes);
    void remove(const QString &suffixes, const QString &name);
    void clear();
    QStringList suffixes();

    Q3PtrList<CodeTemplate> allTemplates() const;

private:
    QMap<QString, QMap<QString, CodeTemplate* > > templates;
    Q3PtrList<CodeTemplate> allCodeTemplates;
    QStringList m_suffixes;
};

class AbbrevPart : public KDevPlugin
{
    Q_OBJECT

public:
    AbbrevPart( QObject *parent, const char *name, const QStringList & );
    ~AbbrevPart();

    bool autoWordCompletionEnabled() const;
    void setAutoWordCompletionEnabled( bool enabled );

    void addTemplate(const QString &templ, const QString &descr,
                     const QString &suffixes, const QString &code);
    void removeTemplate(const QString &suffixes, const QString &name);
    void clearTemplates();
    CodeTemplateList templates() const;

private slots:
    void slotExpandText();
    void slotExpandAbbrev();
    void configWidget(KDialogBase *dlg);
    void slotActivePartChanged( KParts::Part* );
    void slotTextChanged();
    void slotCompletionAborted();
    void slotCompletionDone();
    void slotFilterInsertString( KTextEditor::CompletionEntry*, QString* );
    void slotAboutToShowCompletionBox();

private:
    void updateActions();
    void load();
    void save();
    QString currentWord() const;
    Q3ValueList<KTextEditor::CompletionEntry> findAllWords(const QString &text, const QString &prefix);
    void insertChars( const QString &chars );
//    QAsciiDict<CodeTemplate> m_templates;
    CodeTemplateList m_templates;
    bool m_inCompletion;
    int m_prevLine;
    int m_prevColumn;
    int m_sequenceLength;
    bool m_autoWordCompletionEnabled;
    QString m_completionFile;

    KTextEditor::Document* docIface;
    KTextEditor::EditInterface* editIface;
    KTextEditor::ViewCursorInterface* viewCursorIface;
    KTextEditor::CodeCompletionInterface* completionIface;
};

#endif
