/* $Id$
 *
 *  Copyright (C) 2001 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "qeditor_part.h"
#include "qeditor_view.h"
#include "qeditor.h"
#include "paragdata.h"
#include "settingsdialog.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <private/qrichtext_p.h>

#include "qeditor_part.moc"

struct HLMode{
    QString name;
    QString section;
    QStringList extensions;
};


class CursorImpl: public KTextEditor::Cursor
{
public:
    CursorImpl( QTextDocument* doc ): m_doc( doc ) {
        m_cursor = new QTextCursor( doc );
    }

    virtual ~CursorImpl(){
        delete( m_cursor );
    }

    virtual void position ( unsigned int *line, unsigned int *col ) const{
        *line = m_cursor->parag()->paragId();
        *col = m_cursor->index();
    }

    virtual bool setPosition ( unsigned int line, unsigned int col ){
        m_cursor->setParag( m_cursor->document()->paragAt( line ) );
        m_cursor->setIndex( col );
        return TRUE;
    }

    virtual bool insertText ( const QString& text ){
        m_cursor->parag()->insert( m_cursor->index(), text );
        return TRUE;
    }

    virtual bool removeText ( unsigned int numberOfCharacters ){
        m_cursor->parag()->remove( m_cursor->index(), numberOfCharacters );
        return TRUE;
    }

    virtual QChar currentChar () const{
        return m_cursor->parag()->at( m_cursor->index() )->c;
    }

private:
    QTextDocument* m_doc;
    QTextCursor* m_cursor;
};


K_EXPORT_COMPONENT_FACTORY( libqeditorpart, QEditorPartFactory );

using namespace std;

QEditorPart::QEditorPart( QWidget *parentWidget, const char *widgetName,
                          QObject *parent, const char *name,
                          const QStringList & /*args*/ )
    : KTextEditor::Document( parent, name )
{
    kdDebug() << "QEditorPart::QEditorPart()" << endl;
    // we need an instance
    setInstance( QEditorPartFactory::instance() );

    m_editor = (QEditorView*) createView( parentWidget, widgetName );
    m_editor->editor()->show();
    setWidget( m_editor );

    setupHighlighting();

    // connections
    connect( m_editor->editor(), SIGNAL(textChanged()),
             this, SIGNAL(textChanged()) );
    connect( m_editor->editor(), SIGNAL(selectionChanged()),
             this, SIGNAL(selectionChanged()) );

    setupActions();

    // set our XML-UI resource file
    setXMLFile("qeditor_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

    readConfig();
}

QEditorPart::~QEditorPart()
{
    writeConfig();
}

void QEditorPart::setupActions()
{
    // create our actions
    KStdAction::open( this, SLOT(fileOpen()), actionCollection() );
    KStdAction::saveAs( this, SLOT(fileSaveAs()), actionCollection() );
    KStdAction::save( this, SLOT(save()), actionCollection() );

    KStdAction::undo( this, SLOT(undo()), actionCollection() );
    KStdAction::redo( this, SLOT(redo()), actionCollection() );

    KStdAction::cut( m_editor, SLOT(cut()), actionCollection() );
    KStdAction::copy( m_editor, SLOT(copy()), actionCollection() );
    KStdAction::paste( m_editor, SLOT(paste()), actionCollection() );
    KStdAction::selectAll( m_editor, SLOT(selectAll()), actionCollection() );

    KStdAction::gotoLine( m_editor, SLOT(gotoLine()), actionCollection() );
    KStdAction::find( m_editor, SLOT(doFind()), actionCollection() );
    KStdAction::replace( m_editor, SLOT(doReplace()), actionCollection() );

    new KAction( i18n("&Indent"), CTRL + Key_I,
		 m_editor, SLOT(indent()),
                 actionCollection(), "edit_indent" );

    new KAction( i18n("&Configure Editor..."), 0,
		 this, SLOT(configDialog()),
                 actionCollection(), "settings_configure_editor" );
}

void QEditorPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_editor->editor()->setReadOnly(!rw);
    if (rw)
        connect(m_editor->editor(), SIGNAL(textChanged()),
                this, SLOT(setModified()));
    else
    {
        disconnect(m_editor->editor(), SIGNAL(textChanged()),
                   this, SLOT(setModified()));
    }

    ReadWritePart::setReadWrite(rw);
}

void QEditorPart::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    m_editor->editor()->setModified( modified );

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

KAboutData *QEditorPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)
    KAboutData *aboutData = new KAboutData("qeditorpart", I18N_NOOP("Qt Designer Based Text Editor"), "0.1");
    aboutData->addAuthor("Roberto Raggi", 0, "raggi@cli.di.unipi.it");
    aboutData->addAuthor("Trolltech AS", 0, "info@trolltech.com");
    return aboutData;
}

void QEditorPart::readConfig()
{
    KConfig* config = QEditorPartFactory::instance()->config();
    config->setGroup( "QEditor Part" );
    readConfig( config );
    config->sync();
}

void QEditorPart::writeConfig()
{
    KConfig* config = QEditorPartFactory::instance()->config();
    config->setGroup( "QEditor Part" );
    writeConfig( config );
    config->sync();
}

void QEditorPart::readConfig( KConfig* config )
{
    m_editor->setMarkerWidgetVisible( config->readBoolEntry( "ShowMarkerWidget", false ) );
    m_editor->setLineNumberWidgetVisible( config->readBoolEntry( "ShowLineNumberWidget", true ) );
    m_editor->setLevelWidgetVisible( config->readBoolEntry( "ShowLevelWidget", true ) );
    m_editor->setTabStop( config->readNumEntry( "TabStop", 8 ) );
}

void QEditorPart::writeConfig( KConfig* config )
{
    config->writeEntry( "ShowMarkerWidget", m_editor->isMarkerWidgetVisible() );
    config->writeEntry( "ShowLineNumberWidget", m_editor->isLineNumberWidgetVisible() );
    config->writeEntry( "ShowLevelWidget", m_editor->isLevelWidgetVisible() );
    config->writeEntry( "TabStop", m_editor->tabStop() );
}

bool QEditorPart::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(IO_ReadOnly) == false)
        return false;

    // our example widget is text-based, so we use QTextStream instead
    // of a raw QDataStream
    QTextStream stream(&file);
    QString str;
    while (!stream.eof())
        str += stream.readLine() + "\n";

    file.close();

    m_editor->editor()->setText( str );
    int hl = findMode( m_file );
    setHlMode( hl>=0 ? hl : 0 );

    setModified( false );
    emit fileNameChanged();

    return true;
}

bool QEditorPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    // m_file is always local, so we use QFile
    QFile file(m_file);
    if (file.open(IO_WriteOnly) == false)
        return false;

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    stream << m_editor->editor()->text();

    file.close();

    int hl = findMode( m_file );
    setHlMode( hl>=0 ? hl : 0 );

    setModified( false );
    emit fileNameChanged();

    return true;
}

void QEditorPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName();

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

void QEditorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName();
    if (file_name.isEmpty() == false)
        saveAs(file_name);
}

// -- EditInterface Implementation -- START -------------------------------------------------------------
QString QEditorPart::text() const
{
    return m_editor->editor()->text();
}

QString QEditorPart::text( unsigned int startLine, unsigned int startCol,
                           unsigned int endLine, unsigned int endCol ) const
{
    int selNum = 1000;
    QTextDocument* textDoc = m_editor->editor()->document();

    m_editor->editor()->setSelection( startLine, startCol, endLine, endCol, selNum );
    QString txt = textDoc->selectedText( selNum );
    textDoc->removeSelection( selNum );

    return textDoc->selectedText( selNum );
}

QString QEditorPart::textLine( unsigned int line ) const
{
    return m_editor->editor()->textLine( line );
}

unsigned int QEditorPart::numLines() const
{
    return m_editor->editor()->lines();
}

unsigned int QEditorPart::length() const
{
	return m_editor->editor()->length();
}

int QEditorPart::lineLength( unsigned int line ) const
{
    if( int(line) < m_editor->editor()->lines() ){
        return m_editor->editor()->paragraphLength( line );
    }
    return -1;
}

bool QEditorPart::setText( const QString &text )
{
    m_editor->editor()->setText( text );
    return TRUE;
}

bool QEditorPart::clear()
{
    m_editor->editor()->clear();
    return TRUE;
}

bool QEditorPart::insertText( unsigned int line, unsigned int col, const QString &text )
{
    m_editor->editor()->insertAt( text, line, col );
    return TRUE;
}

bool QEditorPart::removeText( unsigned int startLine, unsigned int startCol,
                              unsigned int endLine, unsigned int endCol )
{
    int selNum = 10;
    m_editor->editor()->setSelection( startLine, startCol, endLine, endCol, selNum );
    m_editor->editor()->removeSelectedText( selNum );
    return TRUE;
}

bool QEditorPart::insertLine( unsigned int line, const QString &text )
{
    m_editor->editor()->insertParagraph( text, line );
    return TRUE;
}

bool QEditorPart::removeLine( unsigned int line )
{
    m_editor->editor()->removeParagraph( line );
    return TRUE;
}

KTextEditor::View* QEditorPart::createView( QWidget *parent, const char *name )
{
    KTextEditor::View* pView = new QEditorView( this, parent, name );
    m_views.append( pView );
    return pView;
}

QPtrList<KTextEditor::View> QEditorPart::views() const
{
    return m_views;
}

// UndoInterface -------------------------------------------------------------------------
void QEditorPart::clearUndo()
{
#warning "TODO: void QEditorPart::clearUndo()"
    kdDebug() << "QEditorPart::clearUndo() -- not implemented yet!!" << endl;
}

void QEditorPart::clearRedo()
{
#warning "TODO: void QEditorPart::clearRedo()"
    kdDebug() << "QEditorPart::clearRedo() -- not implemented yet!!" << endl;
}

unsigned int QEditorPart::undoCount() const
{
#warning "TODO: unsigned int QEditorPart::undoCount() const"
    kdDebug() << "QEditorPart::undoCount() -- not implemented yet!!" << endl;
    return 0;
}

unsigned int QEditorPart::redoCount() const
{
#warning "TODO: unsigned int QEditorPart::redoCount() const"
    kdDebug() << "QEditorPart::redoCount() -- not implemented yet!!" << endl;
    return 0;
}

unsigned int QEditorPart::undoSteps() const
{
    QTextDocument* textDoc = m_editor->editor()->document();
    return textDoc->commands()->undoDepth();
}

void QEditorPart::setUndoSteps( unsigned int steps )
{
    QTextDocument* textDoc = m_editor->editor()->document();
    textDoc->commands()->setUndoDepth( steps );
}

void QEditorPart::undo()
{
    m_editor->editor()->undo();
}

void QEditorPart::redo()
{
    m_editor->editor()->redo();
}

KTextEditor::Cursor* QEditorPart::createCursor( )
{
    KTextEditor::Cursor* c = new CursorImpl( m_editor->editor()->document() );
    m_cursors.append( c );
    return c;
}

QPtrList<KTextEditor::Cursor> QEditorPart::cursors() const
{
    return m_cursors;
}

bool QEditorPart::setSelection( unsigned int startLine, unsigned int startCol,
                                unsigned int endLine, unsigned int endCol )
{
    m_editor->editor()->setSelection( startLine, startCol, endLine, endCol );
    return TRUE;
}

bool QEditorPart::clearSelection()
{
    m_editor->editor()->removeSelection();
    return TRUE;
}

bool QEditorPart::hasSelection() const
{
    return m_editor->editor()->hasSelectedText();
}

QString QEditorPart::selection() const
{
    return m_editor->editor()->selectedText();
}

bool QEditorPart::removeSelectedText()
{
    m_editor->editor()->removeSelectedText();
    return TRUE;
}

bool QEditorPart::selectAll()
{
    m_editor->editor()->selectAll();
    return TRUE;
}

bool QEditorPart::isModified() const
{
    return m_editor->editor()->isModified();
}

void QEditorPart::setupHighlighting()
{
    m_currentMode = 0;

    HLMode* mode = 0;
    m_modes.setAutoDelete( TRUE );

    mode = new HLMode;
    mode->name = "normal";
    mode->section = "Normal";
    mode->extensions = QStringList() << "*.txt" << "*.doc";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "c++";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.cpp" << "*.cxx" << "*.cc" << "*.C" << "*.c++" << "*.c" <<
                       "*.h" << "*.hh" << "*.hxx" << "*.h++" << "*.H";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "java";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.java";
    m_modes.append( mode );

#if defined(HAVE_PERL_MODE)
    mode = new HLMode;
    mode->name = "perl";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.pl";
    m_modes.append( mode );
#endif

    mode = new HLMode;
    mode->name = "python";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.py" << "*.pyw";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "xml";
    mode->section = "Markup";
    mode->extensions = QStringList() << "*.xml" << "*.xslt" << "*.rc" << "*rc" << "*.ui";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "qmake";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.pro";
    m_modes.append( mode );
}

unsigned int QEditorPart::hlMode()
{
    return m_currentMode;
}

bool QEditorPart::setHlMode(unsigned int mode)
{
    if( m_currentMode != mode ){
        m_currentMode = mode;
        HLMode* m = m_modes.at( m_currentMode );
        if( m ){
            m_editor->setLanguage( m->name );
        }
        emit hlChanged();
    }
    return TRUE;
}

unsigned int QEditorPart::hlModeCount()
{
    return m_modes.count();
}

QString QEditorPart::hlModeName(unsigned int mode)
{
    HLMode* m = m_modes.at( mode );
    return m ? m->name : QString::null;
}

QString QEditorPart::hlModeSectionName(unsigned int mode)
{
    HLMode* m = m_modes.at( mode );
    return m ? m->section : QString::null;
}

int QEditorPart::findMode( const QString& filename )
{
    for( unsigned int modeNum=0; modeNum<m_modes.count(); ++modeNum ){
        HLMode* mode = m_modes.at( modeNum );
        QStringList::Iterator itExt = mode->extensions.begin();
        while( itExt != mode->extensions.end() ){
            if( QRegExp(*itExt, TRUE, TRUE).exactMatch(filename) ){
                return modeNum;
            }
            ++itExt;
        }
    }
    return -1;
}

bool QEditorPart::searchText (unsigned int startLine, unsigned int startCol,
                              const QString &text, unsigned int *foundAtLine, unsigned int *foundAtCol,
                              unsigned int *matchLen, bool casesensitive, bool backwards )
{
#warning "TODO: QEditorPart::searchText()"
    kdDebug() << "TODO: QEditorPart::searchText()" << endl;
    return false;
}

bool QEditorPart::searchText (unsigned int startLine, unsigned int startCol,
			const QRegExp &regexp, unsigned int *foundAtLine,
			unsigned int *foundAtCol, unsigned int *matchLen, bool backwards )
{
    QEditor* ed = m_editor->editor();
    QTextParag* p = ed->document()->paragAt( startLine );
    while( p ){
        QString str = p->string()->toString();
        int pos = -1;
        if( backwards ){
            pos = regexp.searchRev( str, p->paragId() == startLine ? startCol : p->length() );
        } else {
            pos = regexp.search( str, p->paragId() == startLine ? startCol : 0 );
        }

        if( pos != -1 ){
            *foundAtLine = p->paragId();
            *foundAtCol = pos;
            *matchLen = regexp.matchedLength();
            return true;
        }

        if( backwards ){
            p = p->prev();
        } else {
            p = p->next();
        }
    }
    return false;
}

uint QEditorPart::mark (uint line)
{
    QTextDocument* textDoc = m_editor->editor()->document();
    QTextParag* parag = textDoc->paragAt( line );
    if( parag ){
        ParagData* data = (ParagData*) parag->extraData();
        if( data ){
            return data->mark();
        }
    }
    return 0;
}

void QEditorPart::setMark (uint line, uint markType)
{
    QTextDocument* textDoc = m_editor->editor()->document();
    QTextParag* parag = textDoc->paragAt( line );
    if( parag ){
        ParagData* data = (ParagData*) parag->extraData();
        if( data ){
            data->setMark( markType );
            emit marksChanged();
        }
    }
}

void QEditorPart::clearMark (uint line)
{
    setMark( line, 0 );
}

void QEditorPart::addMark (uint line, uint markType)
{
    QTextDocument* textDoc = m_editor->editor()->document();
    QTextParag* parag = textDoc->paragAt( line );
    if( parag ){
        ParagData* data = (ParagData*) parag->extraData();
        if( data ){
            data->setMark( data->mark() | markType );
            emit marksChanged();
        }
    }
}

void QEditorPart::removeMark (uint line, uint markType)
{
    QTextDocument* textDoc = m_editor->editor()->document();
    QTextParag* parag = textDoc->paragAt( line );
    if( parag ){
        ParagData* data = (ParagData*) parag->extraData();
        if( data ){
            data->setMark( data->mark() & ~markType );
            emit marksChanged();
        }
    }
}

QPtrList<KTextEditor::Mark> QEditorPart::marks ()
{
    QPtrList<KTextEditor::Mark> marks;
    marks.setAutoDelete( TRUE );
    QTextDocument* textDoc = m_editor->editor()->document();
    QTextParag* p = textDoc->firstParag();
    while( p ){
        ParagData* data = (ParagData*) p->extraData();
        if( data && data->mark() ){
            KTextEditor::Mark* mark = new KTextEditor::Mark;
            mark->type = data->mark();
            mark->line = p->paragId();
            marks.append( mark );
        }
        p = p->next();
    }
    return marks;
}

void QEditorPart::clearMarks ()
{
    QTextDocument* textDoc = m_editor->editor()->document();
    QTextParag* p = textDoc->firstParag();
    while( p ){
        ParagData* data = (ParagData*) p->extraData();
        if( data ){
            data->setMark( 0 );
        }
        p = p->next();
    }
}

void QEditorPart::configDialog()
{
    SettingsDialog dlg;
    dlg.setEditor( this );
    dlg.exec();
}
