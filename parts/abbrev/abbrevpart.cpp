/***************************************************************************
 *   Copyright (C) 2002 Roberto Raggi                                      *
 *   raggi@cli.di.unipi.it                                                 *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "abbrevpart.h"

#include <qfile.h>
#include <qregexp.h>
#include <qvbox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <kaction.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "abbrevconfigwidget.h"


class AbbrevFactory : public KGenericFactory<AbbrevPart>
{
public:
    AbbrevFactory()
        : KGenericFactory<AbbrevPart>( "kdevabbrev" )
    { }

    virtual KInstance *createInstance()
    {
        KInstance *instance = KGenericFactory<AbbrevPart>::createInstance();
        KStandardDirs *dirs = instance->dirs();
        dirs->addResourceType( "codetemplates", 
                               KStandardDirs::kde_default( "data" ) + "kdevabbrev/templates/" );
        return instance;
    }
};

K_EXPORT_COMPONENT_FACTORY( libkdevabbrev, AbbrevFactory );

AbbrevPart::AbbrevPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name)
{
    setInstance(AbbrevFactory::instance());
    setXMLFile("kdevabbrev.rc");

    connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

    KAction *action;
    action = new KAction( i18n("Expand Text"), CTRL + Key_J,
                          this, SLOT(slotExpandText()),
                          actionCollection(), "edit_expandtext" );
    action->setStatusText( i18n("Expand current word") );
    action->setWhatsThis( i18n("Expand current word") );
    
    action = new KAction( i18n("Expand Abbreviation"), SHIFT + Key_Space,
                          this, SLOT(slotExpandAbbrev()),
                          actionCollection(), "edit_expandabbrev" );

    load();
}


AbbrevPart::~AbbrevPart()
{
    save();
}


void AbbrevPart::load()
{
    KStandardDirs *dirs = AbbrevFactory::instance()->dirs();
    QStringList files = dirs->findAllResources("codetemplates", QString::null, false, true);

    QStringList::ConstIterator it;
    for (it = files.begin(); it != files.end(); ++it) {
        QString fn = *it;
        kdDebug(9028) << "fn = " << fn << endl;
        QFile f( fn );
        if ( f.open(IO_ReadOnly) ) {
            QDomDocument doc;
            doc.setContent( &f );
            QDomElement root = doc.firstChild().toElement();
            QDomElement e = root.firstChild().toElement();
            while ( !e.isNull() ){
                addTemplate( e.attribute("name"),
                             e.attribute("description"),
                             e.attribute("suffixes"),
                             e.attribute("code") );
                e = e.nextSibling().toElement();
            }
            f.close();
        }
    }
}


void AbbrevPart::save()
{
    QString fn = AbbrevFactory::instance()->dirs()->saveLocation("data", "codetemplates", true);
    kdDebug(9028) << "fn = " << fn << endl;

    QDomDocument doc( "Templates" );
    QAsciiDictIterator<CodeTemplate> it( m_templates );
    QDomElement root = doc.createElement( "Templates" );
    doc.appendChild( root );
    while( it.current() ){
        CodeTemplate* templ = it.current();
        QDomElement e = doc.createElement( "Template" );
        e.setAttribute( "name", QString::fromLatin1(it.currentKey()) );
        e.setAttribute( "description", templ->description );
        e.setAttribute( "suffixes", templ->suffixes );
        e.setAttribute( "code", templ->code );
        root.appendChild( e );
        ++it;
    }

    QFile f( fn );
    if( f.open(IO_WriteOnly) ){
        QTextStream stream( &f );
        stream << doc.toString();
        f.close();
    }
}


static QString currentWord(KTextEditor::EditInterface *editiface,
                           KTextEditor::ViewCursorInterface *cursoriface)
{
    uint line, col;
    cursoriface->cursorPositionReal(&line, &col);
    QString str = editiface->textLine(line);
    int i;
    for (i = col-1; i >= 0; --i)
        if (!str[i].isLetter())
            break;

    return str.mid(i+1, col-i-1);
}


void AbbrevPart::configWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Abbreviations"));
    AbbrevConfigWidget *w = new AbbrevConfigWidget(this, vbox, "abbrev config widget");
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}


void AbbrevPart::slotExpandText()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    if (!part || !part->widget()) {
        kdDebug() << "no rw part" << endl;
        return;
    }
    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(part);
    if (!editiface) {
        kdDebug() << "no editiface" << endl;
        return;
    }
    KTextEditor::ViewCursorInterface *cursoriface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
    if (!cursoriface) {
        kdDebug() << "no viewcursoriface" << endl;
        return;
    }
    KTextEditor::CodeCompletionInterface *completioniface
        = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
    if (!completioniface) {
        kdDebug() << "no codecompletioniface" << endl;
        return;
    }

    QString word = currentWord(editiface, cursoriface);
    kdDebug(9028) << "Expanding text " << word << endl;
    if (word.isEmpty())
        return;

    QValueList<KTextEditor::CompletionEntry> entries = findAllWords(editiface->text(), word);
    if (entries.count() == 0) {
        ; // some statusbar message?
    } else if (entries.count() == 1) {
        uint line, col;
        cursoriface->cursorPositionReal(&line, &col);
        editiface->insertText(line, col, entries[0].text.mid(word.length()));
    } else {
        completioniface->showCompletionBox(entries, word.length());
    }
}


QValueList<KTextEditor::CompletionEntry> AbbrevPart::findAllWords(const QString &text, const QString &prefix)
{
    QMap<QString, bool> map;
    QValueList<KTextEditor::CompletionEntry> entries;
    QRegExp rx( QString("\\b") + prefix + "[a-zA-Z0-9_]+\\b" );

    int idx = 0;
    int pos = 0;
    int len = 0;
    while ( (pos = rx.match(text, idx, &len)) != -1 ) {
	QString word = text.mid(pos, len);
        if (map.find(word) == map.end()) {
            KTextEditor::CompletionEntry e;
            e.text = word;
            entries << e;
            map[ word ] = TRUE;
        }
        idx = pos + len + 1;
    }
    return entries;
}


void AbbrevPart::slotExpandAbbrev()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    if (!part || !part->widget()) {
        kdDebug() << "no rw part" << endl;
        return;
    }

    QString suffix = part->url().url();
    int pos = suffix.findRev('.');
    if (pos != -1)
        suffix.remove(0, pos+1);

    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(part);
    if (!editiface) {
        kdDebug() << "no editiface" << endl;
        return;
    }
    KTextEditor::ViewCursorInterface *cursoriface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
    if (!cursoriface) {
        kdDebug() << "no viewcursoriface" << endl;
        return;
    }

    QString word = currentWord(editiface, cursoriface);
    kdDebug(9028) << "Expanding word " << word << " with suffix " << suffix << "." << endl;

    QAsciiDictIterator<CodeTemplate> it(m_templates);
    for (; it.current(); ++it) {
        if (it.currentKey() != word)
            continue;
        QString suffixes = it.current()->suffixes;
        int pos = suffixes.find('(');
        if (pos == -1)
            continue;
        suffixes.remove(0, pos+1);
        pos = suffixes.find(')');
        if (pos == -1)
            continue;
        suffixes.remove(pos, suffixes.length()-pos);
        QStringList suffixList = QStringList::split(",", suffixes);
        if (!suffixList.contains(suffix))
            continue;

        uint line, col;
        cursoriface->cursorPositionReal(&line, &col);
        editiface->removeText( line, col-word.length(), line, col );
        insertChars(editiface, cursoriface, it.current()->code );
    }
}


void AbbrevPart::insertChars(KTextEditor::EditInterface *editiface,
                             KTextEditor::ViewCursorInterface *cursoriface,
                             const QString &chars)
{
    bool bMoveCursor = false;
    unsigned int line=0, col=0;
    unsigned int currentLine=0, currentCol=0;

    cursoriface->cursorPositionReal( &currentLine, &currentCol );

    QString spaces;
    QString s = editiface->textLine( currentLine );
    uint i=0;
    while( i<s.length() && s[ i ].isSpace() ){
        spaces += s[ i ];
        ++i;
    }

    QStringList l = QStringList::split( "\n", chars, true );
    for( int i=0; i<(int)l.count(); ++i ){
        QString s = l[ i ];
        int idx = s.find( '|' );
        if( idx != -1 ){
            QString tmp = s.left( idx );
            editiface->insertText( currentLine, currentCol, tmp );
            currentCol += tmp.length();

            line = currentLine;
            col = currentCol;
            bMoveCursor = true;

            tmp = s.mid( idx + 1 );
            editiface->insertText( currentLine, currentCol, tmp );
            currentCol += tmp.length();
        } else {
            editiface->insertText( currentLine, currentCol, s );
            currentCol += s.length();
        }

        if( i != (int)l.count()-1 ){
            editiface->insertLine( ++currentLine, spaces );
            currentCol = spaces.length();
            //pView->keyReturn();
        }
    }

    if( bMoveCursor ){
        cursoriface->setCursorPositionReal( line, col );
    }
}


void AbbrevPart::addTemplate( const QString& templ,
                              const QString& descr,
                              const QString& suffixes,
                              const QString& code)
{
    CodeTemplate* t = m_templates.find( templ );
    if( !t ){
        t = new CodeTemplate();
        m_templates.insert( templ, t );
    }
    t->description = descr;
    t->suffixes = suffixes;
    t->code = code;
}


void AbbrevPart::removeTemplate( const QString& templ )
{
    m_templates.remove( templ );
}


void AbbrevPart::clearTemplates()
{
    m_templates.clear();
}

QAsciiDictIterator<CodeTemplate> AbbrevPart::templates() const
{
    return QAsciiDictIterator<CodeTemplate>( m_templates );
}

#include "abbrevpart.moc"
