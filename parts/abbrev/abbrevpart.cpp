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

#include "abbrevpart.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <QTextStream>
#include <Q3PtrList>
#include <Q3ValueList>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kstandarddirs.h>
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kconfig.h>
#include <kio/netaccess.h>
#include <kiconloader.h>
#include <kdevplugininfo.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "abbrevconfigwidget.h"

static const KDevPluginInfo data("kdevabbrev");

class AbbrevFactory : public KDevGenericFactory<AbbrevPart>
{
public:
    AbbrevFactory()
        : KDevGenericFactory<AbbrevPart>( data )
    { }

    virtual KInstance *createInstance()
    {
        KInstance *instance = KDevGenericFactory<AbbrevPart>::createInstance();
        KStandardDirs *dirs = instance->dirs();
        dirs->addResourceType( "codetemplates",
                               KStandardDirs::kde_default( "data" ) + "kdevabbrev/templates/" );
        dirs->addResourceType( "sources",
                               KStandardDirs::kde_default( "data" ) + "kdevabbrev/sources" );

        return instance;
    }
};

K_EXPORT_COMPONENT_FACTORY( libkdevabbrev, AbbrevFactory )

AbbrevPart::AbbrevPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(&data, parent, name ? name : "AbbrevPart")
{
    setInstance(AbbrevFactory::instance());
    setXMLFile("kdevabbrev.rc");

    connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
    	this, SLOT(slotActivePartChanged(KParts::Part*)) );

    connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

    KAction *action;
    action = new KAction( i18n("Expand Text"), Qt::CTRL + Qt::Key_J,
                          this, SLOT(slotExpandText()),
                          actionCollection(), "edit_expandtext" );
    action->setToolTip( i18n("Expand current word") );
    action->setWhatsThis( i18n("<b>Expand current word</b><p>Current word can be completed using the list of similar words in source files.") );

    action = new KAction( i18n("Expand Abbreviation"), Qt::CTRL + Qt::Key_L,
                          this, SLOT(slotExpandAbbrev()),
                          actionCollection(), "edit_expandabbrev" );
    action->setToolTip( i18n("Expand abbreviation") );
    action->setWhatsThis( i18n("<b>Expand abbreviation</b><p>Enable and configure abbreviations in <b>KDevelop Settings</b>, <b>Abbreviations</b> tab.") );

    load();

    m_inCompletion = false;
    docIface = 0;
    editIface = 0;
    viewCursorIface = 0;
    completionIface = 0;

    m_prevLine = -1;
    m_prevColumn = -1;
    m_sequenceLength = 0;

    KConfig* config = AbbrevFactory::instance()->config();
    KConfigGroupSaver group( config, "General" );
    m_autoWordCompletionEnabled = config->readBoolEntry( "AutoWordCompletion", false );

    updateActions();

    slotActivePartChanged( partController()->activePart() );
}


AbbrevPart::~AbbrevPart()
{
    save();
}

bool AbbrevPart::autoWordCompletionEnabled() const
{
    return m_autoWordCompletionEnabled;
}

void AbbrevPart::setAutoWordCompletionEnabled( bool enabled )
{
    if( enabled == m_autoWordCompletionEnabled )
	return;

    KConfig* config = AbbrevFactory::instance()->config();
    KConfigGroupSaver group( config, "General" );

    m_autoWordCompletionEnabled = enabled;
    config->writeEntry( "AutoWordCompletion", m_autoWordCompletionEnabled );
    config->sync();

    if( !docIface || !docIface->widget() )
	return;

    disconnect( docIface, 0, this, 0 );
    disconnect( docIface->widget(), 0, this, 0 );

    if( m_autoWordCompletionEnabled ){
	connect( docIface->widget(), SIGNAL(completionAborted()),
		 this, SLOT(slotCompletionAborted()) );
	connect( docIface->widget(), SIGNAL(completionDone()),
		 this, SLOT(slotCompletionDone()) );
	connect( docIface->widget(), SIGNAL(aboutToShowCompletionBox()),
		 this, SLOT(slotAboutToShowCompletionBox()) );

	connect( docIface, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    }
}
void AbbrevPart::load()
{
    KStandardDirs *dirs = AbbrevFactory::instance()->dirs();
    QString localTemplatesFile = locateLocal("codetemplates", "templates", AbbrevFactory::instance());
    QStringList files;
    if (QFileInfo(localTemplatesFile).exists())
        files << localTemplatesFile;
    else
        files = dirs->findAllResources("codetemplates", QString::null, false, true);

    QString localSourcesFile = locateLocal("sources", "sources", AbbrevFactory::instance());
    QStringList sourceFiles;
    if (QFileInfo(localSourcesFile).exists())
        sourceFiles << localSourcesFile;
    else
        sourceFiles = dirs->findAllResources("sources", QString::null, false, true);
    kdDebug(9028) << "=========> sourceFiles: " << sourceFiles.join(" ") << endl;

    this->m_completionFile = QString::null;
    for( QStringList::Iterator it=sourceFiles.begin(); it!=sourceFiles.end(); ++it ) {
        QString fn = *it;
	kdDebug(9028) << "===> load file: " << fn << endl;
        QFile f( fn );
        if ( f.open(QIODevice::ReadOnly) ) {
		QTextStream stream( &f );
		m_completionFile += ( stream.read() + QString("\n") );
		f.close();
	}
    }

    QStringList::ConstIterator it;
    for (it = files.begin(); it != files.end(); ++it) {
        QString fn = *it;
        kdDebug(9028) << "fn = " << fn << endl;
        QFile f( fn );
        if ( f.open(QIODevice::ReadOnly) ) {
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
    QString fn = AbbrevFactory::instance()->dirs()->saveLocation("codetemplates", "", true);
    kdDebug(9028) << "fn = " << fn << endl;

    QDomDocument doc( "Templates" );
    QDomElement root = doc.createElement( "Templates" );
    doc.appendChild( root );

    Q3PtrList<CodeTemplate> templates = m_templates.allTemplates();
    CodeTemplate *templ;
    for (templ = templates.first(); templ; templ = templates.next())
    {
        QDomElement e = doc.createElement( "Template" );
        e.setAttribute( "name", templ->name );
        e.setAttribute( "description", templ->description );
        e.setAttribute( "suffixes", templ->suffixes );
        e.setAttribute( "code", templ->code );
        root.appendChild( e );
    }

    QFile f( fn + "templates" );
    if( f.open(QIODevice::WriteOnly) ){
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
	Q3VBox *vbox = dlg->addVBoxPage(i18n("Abbreviations"), i18n("Abbreviations"), BarIcon( info()->icon(), KIcon::SizeMedium) );
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

    Q3ValueList<KTextEditor::CompletionEntry> entries = findAllWords(editIface->text(), word);
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


Q3ValueList<KTextEditor::CompletionEntry> AbbrevPart::findAllWords(const QString &text, const QString &prefix)
{
    Q3ValueList<KTextEditor::CompletionEntry> entries;

    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    QWidget *view = partController()->activeWidget();
    if (!part || !view) {
        kdDebug(9028) << "no rw part" << endl;
        return entries;
    }

    QString suffix = part->url().url();
    int pos = suffix.findRev('.');
    if (pos != -1)
        suffix.remove(0, pos+1);
    kdDebug(9028) << "AbbrevPart::findAllWords with suffix " << suffix << endl;

    QMap<QString, bool> map;
    QRegExp rx( QString("\\b") + prefix + "[a-zA-Z0-9_]+\\b" );

    int idx = 0;
    pos = 0;
    int len = 0;
    while ( (pos = rx.search(text, idx)) != -1 ) {
	len = rx.matchedLength();
	QString word = text.mid(pos, len);
        if (map.find(word) == map.end()) {
            KTextEditor::CompletionEntry e;
            e.text = word;
            entries << e;
            map[ word ] = TRUE;
        }
        idx = pos + len + 1;
    }

    idx = 0;
    pos = 0;
    len = 0;
    while ( (pos = rx.search(m_completionFile, idx)) != -1 ) {
	len = rx.matchedLength();
	QString word = m_completionFile.mid(pos, len);
        if (map.find(word) == map.end()) {
            KTextEditor::CompletionEntry e;
            e.text = word;
            entries << e;
            map[ word ] = TRUE;
        }
        idx = pos + len + 1;
    }


    QMap<QString, CodeTemplate*> m = m_templates[suffix];
    for (QMap<QString, CodeTemplate*>::const_iterator it = m.begin(); it != m.end() ; ++it) {
        KTextEditor::CompletionEntry e;
        e.text = it.data()->description + " <abbrev>";
        e.userdata = it.key();
        entries << e;
    }

    return entries;
}


void AbbrevPart::slotExpandAbbrev()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    QWidget *view = partController()->activeWidget();
    if (!part || !view) {
        kdDebug(9028) << "no rw part" << endl;
        return;
    }

    QString suffix = part->url().url();
    int pos = suffix.findRev('.');
    if (pos != -1)
        suffix.remove(0, pos+1);

    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(part);
    if (!editiface) {
        kdDebug(9028) << "no editiface" << endl;
        return;
    }
    KTextEditor::ViewCursorInterface *cursoriface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
    if (!cursoriface) {
        kdDebug(9028) << "no viewcursoriface" << endl;
        return;
    }

    QString word = currentWord();
    kdDebug(9028) << "Expanding word " << word << " with suffix " << suffix << "." << endl;

    QMap<QString, CodeTemplate*> m = m_templates[suffix];
    for (QMap<QString, CodeTemplate*>::const_iterator it = m.begin(); it != m.end() ; ++it) {
        if (it.key() != word)
            continue;

        uint line, col;
        cursoriface->cursorPositionReal(&line, &col);
        editiface->removeText( line, col-word.length(), line, col );
        insertChars(it.data()->code );
    }
}


void AbbrevPart::insertChars( const QString &chars )
{
    unsigned line=0, col=0;
    viewCursorIface->cursorPositionReal( &line, &col );

    unsigned int currentLine=line, currentCol=col;

    QString spaces;
    QString s = editIface->textLine( currentLine );
    uint i=0;
    while( i<s.length() && s[ i ].isSpace() ){
        spaces += s[ i ];
        ++i;
    }

    bool foundPipe = false;
    QString str;
    QTextStream stream( &str, QIODevice::WriteOnly );
    QStringList lines = QStringList::split( "\n", chars );
    QStringList::Iterator it = lines.begin();
    line = currentLine;
    while( it != lines.end() ){
        QString lineText = *it;
	if( it != lines.begin() ){
            stream << spaces;
	    if( !foundPipe )
		currentCol += spaces.length();
	}

        int idx = lineText.find( '|' );
        if( idx != -1 ){
            stream << lineText.left( idx ) << lineText.mid( idx+1 );
	    if( !foundPipe ){
		foundPipe = true;
		currentCol += lineText.left( idx ).length();
		kdDebug(9007) << "found pipe at " << currentLine << ", " << currentCol << endl;
	    }
        } else {
            stream << lineText;
        }

        ++it;

	if( it != lines.end() ){
            stream << "\n";
	    if( !foundPipe ){
		++currentLine;
		currentCol = 0;
	    }
	}
    }
    editIface->insertText( line, col, str );
    kdDebug(9007) << "go to " << currentLine << ", " << currentCol << endl;
    viewCursorIface->setCursorPositionReal( currentLine, currentCol );
}

void AbbrevPart::addTemplate( const QString& templ,
                              const QString& descr,
                              const QString& suffixes,
                              const QString& code)
{
    m_templates.insert(templ, descr, code, suffixes);
}


void AbbrevPart::removeTemplate( const QString &suffixes, const QString &name )
{
    m_templates.remove( suffixes, name );
}


void AbbrevPart::clearTemplates()
{
    m_templates.clear();
}

CodeTemplateList AbbrevPart::templates() const
{
    return m_templates;
}

void AbbrevPart::slotActivePartChanged( KParts::Part* part )
{
    kdDebug(9028) << "AbbrevPart::slotActivePartChanged()" << endl;
    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );

    if( !doc || !part->widget() || doc == docIface  )
    {
        actionCollection()->action( "edit_expandtext" )->setEnabled( false );
        actionCollection()->action( "edit_expandabbrev" )->setEnabled( false );
        return;
    }

    docIface = doc;

    if( !docIface ){
        docIface = 0;
        editIface = 0;
        viewCursorIface = 0;
        completionIface = 0;
    }

    editIface = dynamic_cast<KTextEditor::EditInterface*>( part );
    viewCursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>( part->widget() );
    completionIface = dynamic_cast<KTextEditor::CodeCompletionInterface*>( part->widget() );

    updateActions();

    if( !editIface || !viewCursorIface || !completionIface )
    	return;

    disconnect( part->widget(), 0, this, 0 );
    disconnect( doc, 0, this, 0 );

    connect( part->widget(), SIGNAL(filterInsertString(KTextEditor::CompletionEntry*, QString*)),
	     this, SLOT(slotFilterInsertString(KTextEditor::CompletionEntry*, QString*)) );

    if( autoWordCompletionEnabled() ){
	connect( part->widget(), SIGNAL(completionAborted()), this, SLOT(slotCompletionAborted()) );
	connect( part->widget(), SIGNAL(completionDone()), this, SLOT(slotCompletionDone()) );
	connect( part->widget(), SIGNAL(aboutToShowCompletionBox()), this, SLOT(slotAboutToShowCompletionBox()) );
	connect( doc, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    }

    m_prevLine = -1;
    m_prevColumn = -1;
    m_sequenceLength = 0;
    kdDebug(9028) << "AbbrevPart::slotActivePartChanged() -- OK" << endl;
}

void AbbrevPart::slotTextChanged()
{
    if( m_inCompletion )
	return;

    unsigned int line, col;
    viewCursorIface->cursorPositionReal( &line, &col );

    if( m_prevLine != int(line) || m_prevColumn+1 != int(col) || col == 0 ){
        m_prevLine = line;
        m_prevColumn = col;
	m_sequenceLength = 1;
	return;
    }

    QString textLine = editIface->textLine( line );
    QChar ch = textLine[ col-1 ];
    QChar currentChar = textLine[ col ];

    if( currentChar.isLetterOrNumber() || currentChar == QChar('_') || !(ch.isLetterOrNumber() || ch == QChar('_')) ){
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
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    QWidget *view = partController()->activeWidget();
    if (!part || !view) {
        kdDebug(9028) << "no rw part" << endl;
        return;
    }

    QString suffix = part->url().url();
    int pos = suffix.findRev('.');
    if (pos != -1)
        suffix.remove(0, pos+1);
    kdDebug(9028) << "AbbrevPart::slotFilterInsertString with suffix " << suffix << endl;

    if( !entry || !text || !viewCursorIface || !editIface )
	return;

    QString expand( " <abbrev>" );
    if( !entry->userdata.isNull() && entry->text.endsWith(expand) ){
	QString macro = entry->text.left( entry->text.length() - expand.length() );
	*text = "";
        uint line, col;
        viewCursorIface->cursorPositionReal( &line, &col );
        editIface->removeText( line, col-currentWord().length(), line, col );
	insertChars( m_templates[suffix][entry->userdata]->code );
    }
}

void AbbrevPart::updateActions()
{
    actionCollection()->action( "edit_expandtext" )->setEnabled( docIface != 0 );
    actionCollection()->action( "edit_expandabbrev" )->setEnabled( docIface != 0 );
}

void AbbrevPart::slotCompletionAborted()
{
    kdDebug(9028) << "AbbrevPart::slotCompletionAborted()" << endl;
    m_inCompletion = false;
}

void AbbrevPart::slotCompletionDone()
{
    kdDebug(9028) << "AbbrevPart::slotCompletionDone()" << endl;
    m_inCompletion = false;
}

void AbbrevPart::slotAboutToShowCompletionBox()
{
    kdDebug(9028) << "AbbrevPart::slotAboutToShowCompletionBox()" << endl;
    m_inCompletion = true;
}

CodeTemplateList::CodeTemplateList( )
{
    allCodeTemplates.setAutoDelete(true);
}

CodeTemplateList::~ CodeTemplateList( )
{
}

QMap< QString, CodeTemplate * > CodeTemplateList::operator [ ]( QString suffix )
{
    kdDebug(9028) << "CodeTemplateList::operator []" << endl;
    QMap< QString, CodeTemplate * > selectedTemplates;
    for (QMap<QString, QMap<QString, CodeTemplate* > >::const_iterator it = templates.begin(); it != templates.end(); ++it)
    {
        kdDebug(9028) << "CodeTemplateList::operator [] - suffixes " << it.key() << endl;
        if (QStringList::split(",", it.key()).contains(suffix))
        {
            kdDebug(9028) << "CodeTemplateList::operator [] - suffixes " << it.key() << " contains " << suffix << endl;

            QMap<QString, CodeTemplate* > m = it.data();
            for (QMap<QString, CodeTemplate* >::const_iterator itt = m.begin(); itt != m.end(); ++itt)
            {
                kdDebug(9028) << "x" << endl;
                selectedTemplates[itt.key()] = itt.data();
            }
        }
    }
    return selectedTemplates;
}

void CodeTemplateList::insert( QString name, QString description, QString code, QString suffixes )
{
    QString origSuffixes = suffixes;
//    QStringList suffixList;
    int pos = suffixes.find('(');
    if (pos == -1)
        return;
    suffixes.remove(0, pos+1);
    pos = suffixes.find(')');
    if (pos == -1)
        return;
    suffixes.remove(pos, suffixes.length()-pos);
//    suffixList = QStringList::split(",", suffixes);

    CodeTemplate *t;
    if (templates.contains(suffixes) && templates[suffixes].contains(name))
    {
        kdDebug(9028) << "found template for suffixes " << suffixes << " and name " << name << endl;
        t = templates[suffixes][name];
    }
    else
    {
        kdDebug(9028) << "creating template for suffixes " << suffixes << " and name " << name << endl;
        t = new CodeTemplate();
        allCodeTemplates.append(t);
        templates[suffixes][name] = t;
    }
    t->name = name;
    t->description = description;
    t->code = code;
    t->suffixes = origSuffixes;
    if (!m_suffixes.contains(origSuffixes))
        m_suffixes.append(origSuffixes);
}

Q3PtrList< CodeTemplate > CodeTemplateList::allTemplates( ) const
{
    return allCodeTemplates;
}

void CodeTemplateList::remove( const QString & suffixes, const QString & name )
{
    allCodeTemplates.remove(templates[suffixes][name]);
    templates[suffixes].remove(name);
}

void CodeTemplateList::clear( )
{
    templates.clear();
    allCodeTemplates.clear();
}

QStringList CodeTemplateList::suffixes( )
{
    return m_suffixes;
}

#include "abbrevpart.moc"
