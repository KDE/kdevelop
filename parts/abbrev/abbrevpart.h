/***************************************************************************
 *   Copyright (C) 2002 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                 *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ABBREVPART_H_
#define _ABBREVPART_H_

#include <qasciidict.h>
#include "kdevplugin.h"
#include "ktexteditor/editinterface.h"
#include "ktexteditor/viewcursorinterface.h"
#include "ktexteditor/codecompletioninterface.h"


class KDialogBase;

struct CodeTemplate {
    QString description;
    QString code;
    QString suffixes;
};


class AbbrevPart : public KDevPlugin
{
    Q_OBJECT

public:
    AbbrevPart( QObject *parent, const char *name, const QStringList & );
    ~AbbrevPart();

    void addTemplate(const QString &templ, const QString &descr,
                     const QString &suffixes, const QString &code);
    void removeTemplate(const QString &templ);
    void clearTemplates();
    QAsciiDictIterator<CodeTemplate> templates() const;
    
private slots:
    void slotExpandText();
    void slotExpandAbbrev();
    void configWidget(KDialogBase *dlg);

private:
    void load();
    void save();
    QValueList<KTextEditor::CompletionEntry> findAllWords(const QString &text, const QString &prefix);
    void insertChars(KTextEditor::EditInterface *editiface,
                     KTextEditor::ViewCursorInterface *cursoriface,
                     const QString &chars);
    QAsciiDict<CodeTemplate> m_templates;
};

#endif
