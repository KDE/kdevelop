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
#include <kconfig.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>

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
    : KDevPlugin(parent, name ? name : "AbbrevPart")
{
    setInstance(AbbrevFactory::instance());
    setXMLFile("kdevabbrev.rc");

    connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
    	this, SLOT(slotActivePartChanged(KParts::Part*)) );
	
    connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

    KAction *action;
    action = new KAction( i18n("Expand Text"), CTRL + Key_J,
                          this, SLOT(slotExpandText()),
                          actionCollection(), "edit_expandtext" );
    action->setStatusText( i18n("Expand current word") );
    action->setWhatsThis( i18n("Expand current word") );

    action = new KAction( i18n("Expand Abbreviation"), CTRL + Key_L,
                          this, SLOT(slotExpandAbbrev()),
                          actionCollection(), "edit_expandabbrev" );

    load();
    
    m_inCompletion = false;
    docIface = 0;
    editIface = 0;
    viewCursorIface = 0;
    completionIface = 0;

    m_prevLine = -1;
    m_prevColumn = -1;
    m_sequenceLength = 0;

    updateActions();
}


AbbrevPart::~AbbrevPart()
{
    save();
}

void AbbrevPart::slotCompletionAborted()
{
    m_inCompletion = false;
}

void AbbrevPart::slotCompletionDone()
{
    m_inCompletion = false;
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
        e.setAttribute( "name", it.currentKey() );
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


QString AbbrevPart::currentWord() const
{
    uint line, col;
    viewCursorIface->cursorPositionReal(&line, &col);
    QString str = editIface->textLine(line);
    int i;
    for (i = col-1; i >= 0; --i)
        if( ! (str[i].isLetter() || str[i] == '_') )
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
    if( !editIface || !completionIface || !viewCursorIface )
        return;
	
    QString word = currentWord();
    if (word.isEmpty())
        return;

    QValueList<KTextEditor::CompletionEntry> entries = findAllWords(editIface->text(), word);
    if (entries.count() == 0) {
        ; // some statusbar message?
//    } else if (entries.count() == 1) {
//        uint line, col;
//        viewCursorIface->cursorPositionReal(&line, &col);
//        QString txt = entries[0].text.mid(word.length());
//        editIface->insertText( line, col, txt );
//        viewCursorIface->setCursorPositionReal( line, col + txt.length() );
    } else {
        m_inCompletion = true;
        completionIface->showCompletionBox(entries, word.length());
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
    
    QAsciiDictIterator<CodeTemplate> it( m_templates );
    while( it.current() ) {
	KTextEditor::CompletionEntry e;
	e.text = it.current()->description + " <abbrev>";
	e.userdata = it.currentKey();
	entries << e;
	++it;
    }
    
    return entries;
}


void AbbrevPart::slotExpandAbbrev()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    QWidget *view = partController()->activeWidget();
    if (!part || !view) {
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
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
    if (!cursoriface) {
        kdDebug() << "no viewcursoriface" << endl;
        return;
    }

    QString word = currentWord();
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
        insertChars(it.current()->code );
    }
}


void AbbrevPart::insertChars( const QString &chars )
{
    bool bMoveCursor = false;
    unsigned int line=0, col=0;
    unsigned int currentLine=0, currentCol=0;

    viewCursorIface->cursorPositionReal( &currentLine, &currentCol );

    QString spaces;
    QString s = editIface->textLine( currentLine );
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
            editIface->insertText( currentLine, currentCol, tmp );
            currentCol += tmp.length();

            line = currentLine;
            col = currentCol;
            bMoveCursor = true;

            tmp = s.mid( idx + 1 );
            editIface->insertText( currentLine, currentCol, tmp );
            currentCol += tmp.length();
        } else {
            editIface->insertText( currentLine, currentCol, s );
            currentCol += s.length();
        }

        if( i != (int)l.count()-1 ){
            editIface->insertLine( ++currentLine, spaces );
            currentCol = spaces.length();
            //pView->keyReturn();
        }
    }

    if( bMoveCursor ){
        viewCursorIface->setCursorPositionReal( line, col );
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

void AbbrevPart::slotActivePartChanged( KParts::Part* part )
{
    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    
    if( doc == docIface )
        return;
	
    docIface = doc;
	
    if( !docIface ){
        docIface = 0;
        editIface = 0;
        viewCursorIface = 0;
        completionIface = 0;
    }

    editIface = dynamic_cast<KTextEditor::EditInterface*>( part );

    QWidget *view = partController()->activeWidget();
    viewCursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>( view );
    completionIface = dynamic_cast<KTextEditor::CodeCompletionInterface*>( view );

    updateActions();

    if( !editIface || !viewCursorIface || !completionIface )
    	return;

    KConfig* config = AbbrevFactory::instance()->config();
    config->setGroup( "General" );

    disconnect( view, 0, this, 0 );
    disconnect( doc, 0, this, 0 );

    connect( view, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*, QString*)),
	     this, SLOT(slotFilterInsertString(KTextEditor::CompletionEntry*, QString*)) );

    if( config->readBoolEntry("AutoExpand", true) ){
	connect( view, SIGNAL(completionAborted()),
		 this, SLOT(slotCompletionAborted()) );
	connect( view, SIGNAL(completionDone()),
		 this, SLOT(slotCompletionDone()) );

	connect( doc, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    }

    m_prevLine = -1;
    m_prevColumn = -1;
    m_sequenceLength = 0;
}

void AbbrevPart::slotTextChanged()
{
    unsigned int line, col;
    viewCursorIface->cursorPositionReal( &line, &col );

    if( m_prevLine != int(line) || m_prevColumn+1 != int(col) || col == 0 ){
        m_prevLine = line;
        m_prevColumn = col;
	m_sequenceLength = 1;
	return;
    }

    QChar ch = editIface->textLine( line )[ col-1 ];

    if( m_inCompletion || !(ch.isLetterOrNumber() || ch == QChar('_')) ){
        // reset
        m_prevLine = -1;
	return;
    }

    if( m_sequenceLength >= 3 )
	slotExpandText();

    ++m_sequenceLength;
    m_prevLine = line;
    m_prevColumn = col;
}

void AbbrevPart::slotFilterInsertString( KTextEditor::CompletionEntry* entry, QString* text )
{
    kdDebug(9028) << "AbbrevPart::slotFilterInsertString()" << endl;

    if( !entry || !text || !viewCursorIface || !editIface )
	return;

    QString expand( " <abbrev>" );
    if( entry->userdata && entry->text.endsWith(expand) ){
	QString macro = entry->text.left( entry->text.length() - expand.length() );
	*text = "";
        uint line, col;
        viewCursorIface->cursorPositionReal( &line, &col );
        editIface->removeText( line, col-currentWord().length(), line, col );
	insertChars( m_templates[entry->userdata]->code );
    }
}

void AbbrevPart::updateActions()
{
    actionCollection()->action( "edit_expandtext" )->setEnabled( docIface != 0 );
    actionCollection()->action( "edit_expandabbrev" )->setEnabled( docIface != 0 );
}

#include "abbrevpart.moc"
