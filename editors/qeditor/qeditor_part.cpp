/*
 *  Copyright (C) 2001 Roberto Raggi (roberto@kdevelop.org)
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

#include <qvbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qlayout.h>
#include <private/qrichtext_p.h>

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kxmlguifactory.h>

#include "qeditor_view.h"
#include "qeditor_settings.h"
#include "qeditor_browserextension.h"
#include "qeditor.h"
#include "paragdata.h"
#include "qsourcecolorizer.h"
#include "qeditor_indenter.h"
#include "highlightingconfigpage.h"
#include "generalconfigpage.h"
#include "markerwidget.h"

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
        *line = m_cursor->paragraph()->paragId();
        *col = m_cursor->index();
    }

    virtual bool setPosition ( unsigned int line, unsigned int col ){
        m_cursor->setParagraph( m_cursor->document()->paragAt( line ) );
        m_cursor->setIndex( col );
        return TRUE;
    }

    virtual bool insertText ( const QString& text ){
        m_cursor->paragraph()->insert( m_cursor->index(), text );
        return TRUE;
    }

    virtual bool removeText ( unsigned int numberOfCharacters ){
        m_cursor->paragraph()->remove( m_cursor->index(), numberOfCharacters );
        return TRUE;
    }

    virtual QChar currentChar () const{
        return m_cursor->paragraph()->at( m_cursor->index() )->c;
    }

private:
    QTextDocument* m_doc;
    QTextCursor* m_cursor;
};

using namespace std;

QEditorPart::QEditorPart( QWidget *parentWidget, const char *widgetName,
                          QObject *parent, const char *name,
                          const QStringList & /*args*/ )
    : KTextEditor::Document( parent, name == 0 ? "QEditorPart" : name ), m_currentView( 0 )
{
    kdDebug(9032) << "QEditorPart::QEditorPart()" << endl;
    // we need an instance
    setInstance( QEditorPartFactory::instance() );

    QEditorPartFactory::registerDocument( this );

    m_views.setAutoDelete( FALSE );
    m_cursors.setAutoDelete( TRUE );
    m_marks.setAutoDelete( TRUE );

    (void) createView( parentWidget, widgetName );
    m_extension = new QEditorBrowserExtension( this );

    setupHighlighting();

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

}

QEditorPart::~QEditorPart()
{
    QEditorPartFactory::deregisterDocument( this );
}

void QEditorPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_currentView->editor()->setReadOnly(!rw);
    if (rw){
        connect(m_currentView->editor(), SIGNAL(textChanged()),
                this, SLOT(setModified()));
    } else {
        disconnect(m_currentView->editor(), SIGNAL(textChanged()),
                   this, SLOT(setModified()));
    }

    ReadWritePart::setReadWrite(rw);
}

void QEditorPart::setModified(bool modified)
{
    if ( modified == ReadWritePart::isModified() )
        return; //nothing to do

    m_currentView->editor()->setModified( modified );

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);

    // get a handle on our Save action and make sure it is valid
    KAction *save = m_currentView->actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (save)
        // if so, we either enable or disable it based on the current
        // state
        save->setEnabled(modified);

    // same for the reload action
    KAction *reload = m_currentView->actionCollection()->action("Reload");
    if ( reload )
        reload->setEnabled(modified);

    emit newStatus();
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

    m_currentView->editor()->setText( str );
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

    if( m_file.isEmpty() ){
	fileSaveAs();
	return true;
    }

    // m_file is always local, so we use QFile
    QFile file(m_file);
    if (file.open(IO_WriteOnly) == false)
        return false;

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    stream << m_currentView->editor()->text();

    file.close();

    int hl = findMode( m_file );
    setHlMode( hl>=0 ? hl : 0 );

    setModified( false );
    emit fileNameChanged();

    return true;
}

void QEditorPart::fileReload()
{
    if (openURL(url())) {
        setModified( false );
	emit newStatus();
    }
}

void QEditorPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName();

    if (file_name.isEmpty() == false)
        openURL(KURL::fromPathOrURL( file_name ));
}

void QEditorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName();
    if (file_name.isEmpty() == false)
        saveAs(KURL::fromPathOrURL( file_name ));
}

// -- EditInterface Implementation -- START -------------------------------------------------------------
QString QEditorPart::text() const
{
    return m_currentView->editor()->text();
}

QString QEditorPart::text( unsigned int startLine, unsigned int startCol,
                           unsigned int endLine, unsigned int endCol ) const
{
    int selNum = 1000;
    QTextDocument* textDoc = m_currentView->editor()->document();

    m_currentView->editor()->setSelection( startLine, startCol, endLine, endCol, selNum );
    QString txt = textDoc->selectedText( selNum );
    textDoc->removeSelection( selNum );

    return textDoc->selectedText( selNum );
}

QString QEditorPart::textLine( unsigned int line ) const
{
    return m_currentView->editor()->textLine( line );
}

unsigned int QEditorPart::numLines() const
{
    return m_currentView->editor()->lines();
}

unsigned int QEditorPart::length() const
{
	return m_currentView->editor()->length();
}

int QEditorPart::lineLength( unsigned int line ) const
{
    if( int(line) < m_currentView->editor()->lines() ){
        return m_currentView->editor()->paragraphLength( line );
    }
    return -1;
}

bool QEditorPart::setText( const QString &text )
{
    m_currentView->editor()->setText( text );
    return TRUE;
}

bool QEditorPart::clear()
{
    m_currentView->editor()->clear();
    return TRUE;
}

bool QEditorPart::insertText( unsigned int line, unsigned int col, const QString &text )
{
    m_currentView->editor()->insertAt( text, line, col );
    return TRUE;
}

bool QEditorPart::removeText( unsigned int startLine, unsigned int startCol,
                              unsigned int endLine, unsigned int endCol )
{
    int selNum = 10;
    m_currentView->editor()->setSelection( startLine, startCol, endLine, endCol, selNum );
    m_currentView->editor()->removeSelectedText( selNum );
    return TRUE;
}

bool QEditorPart::insertLine( unsigned int line, const QString &text )
{
    m_currentView->editor()->insertParagraph( text, line );
    return TRUE;
}

bool QEditorPart::removeLine( unsigned int line )
{
    m_currentView->editor()->removeParagraph( line );
    return TRUE;
}

KTextEditor::View* QEditorPart::createView( QWidget* parentWidget, const char* widgetName )
{
    kdDebug(9032) << "QEditorPart::createView()" << endl;

    if( !m_currentView ){
	m_currentView = new QEditorView( this, parentWidget, widgetName );
	m_views.append( m_currentView );
	insertChildClient( m_currentView );
	setWidget( m_currentView );
    }
    else
	m_currentView->reparent( parentWidget, QPoint(0,0) );

    return m_currentView;
}

QPtrList<KTextEditor::View> QEditorPart::views() const
{
    return m_views;
}

// UndoInterface -------------------------------------------------------------------------
void QEditorPart::clearUndo()
{
#warning "TODO: void QEditorPart::clearUndo()"
    kdDebug(9032) << "QEditorPart::clearUndo() -- not implemented yet!!" << endl;
}

void QEditorPart::clearRedo()
{
#warning "TODO: void QEditorPart::clearRedo()"
    kdDebug(9032) << "QEditorPart::clearRedo() -- not implemented yet!!" << endl;
}

unsigned int QEditorPart::undoCount() const
{
#warning "TODO: unsigned int QEditorPart::undoCount() const"
    kdDebug(9032) << "QEditorPart::undoCount() -- not implemented yet!!" << endl;
    return 0;
}

unsigned int QEditorPart::redoCount() const
{
#warning "TODO: unsigned int QEditorPart::redoCount() const"
    kdDebug(9032) << "QEditorPart::redoCount() -- not implemented yet!!" << endl;
    return 0;
}

unsigned int QEditorPart::undoSteps() const
{
    QTextDocument* textDoc = m_currentView->editor()->document();
    return textDoc->commands()->undoDepth();
}

void QEditorPart::setUndoSteps( unsigned int steps )
{
    QTextDocument* textDoc = m_currentView->editor()->document();
    textDoc->commands()->setUndoDepth( steps );
}

void QEditorPart::undo()
{
    m_currentView->editor()->undo();
}

void QEditorPart::redo()
{
    m_currentView->editor()->redo();
}

KTextEditor::Cursor* QEditorPart::createCursor( )
{
    KTextEditor::Cursor* c = new CursorImpl( m_currentView->editor()->document() );
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
    m_currentView->editor()->setSelection( startLine, startCol, endLine, endCol );
    return TRUE;
}

bool QEditorPart::clearSelection()
{
    m_currentView->editor()->removeSelection();
    return TRUE;
}

bool QEditorPart::hasSelection() const
{
    return m_currentView->editor()->hasSelectedText();
}

QString QEditorPart::selection() const
{
    return m_currentView->editor()->selectedText();
}

bool QEditorPart::removeSelectedText()
{
    m_currentView->editor()->removeSelectedText();
    return TRUE;
}

bool QEditorPart::selectAll()
{
    m_currentView->editor()->selectAll();
    return TRUE;
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
    mode->extensions = QStringList() << "*.cpp" << "*.cxx" << "*.cc" << "*.C" << "*.c++" << "*.ocl" << "*.c" << "*.tcc" <<
                       "*.m" << "*.mm" << "*.M" << "*.inl" << "*.tlh" << "*.diff" << "*.patch" << "*.moc" << "*.xpm" <<
                       "*.h" << "*.hpp" << "*.hh" << "*.hxx" << "*.h++" << "*.H";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "java";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.java";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "javascript";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.js" << "*.qs";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "csharp";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.cs";
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
    mode->extensions = QStringList() << "*.xml" << "*.xslt" << "*.rc" << "*rc" << "*.ui" <<
                                        "*.html" << "*.htm" << "*.kdevelop" << "*.kdevses" ;
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "qmake";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.pro" << "*.sh" << "*Makefile" << "*Makefile.am" << "*Makefile.in";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "jsp";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.jsp";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "ocaml";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.ml" << "*.mli";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "pascal";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.pp" << "*.p" << "*.pas" << "*.dpr";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "ada";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.adb" << "*.ads";
    m_modes.append( mode );

    mode = new HLMode;
    mode->name = "sql";
    mode->section = "Programming";
    mode->extensions = QStringList() << "*.sql" << "*.SQL";
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
            m_currentView->setLanguage( m->name );
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
	/*
    Q_UNUSED( startLine );
    Q_UNUSED( startCol );
    Q_UNUSED( text );
    Q_UNUSED( foundAtLine );
    Q_UNUSED( foundAtCol );
    Q_UNUSED( matchLen );
    Q_UNUSED( casesensitive );
    Q_UNUSED( backwards );
	*/
	if( text.isEmpty() ){
		return false;
	}

	unsigned int tmpline = startLine;
	int foundCol;
	QString oneline;
	bool firstTestedLine=true;

	if( !backwards ){
		while( tmpline <= numLines() ){
			//int oneLineLen = lineLength( tmpline-1 );
			oneline = textLine( tmpline );
			kdDebug()<<oneline<<endl;

			// test one line by one line
			if( firstTestedLine ){
				int index = (int)startCol;
				foundCol = oneline.find( text, index, casesensitive );
				firstTestedLine = false;
			}else{
				foundCol = oneline.find( text, 0, casesensitive );
			}
			//case of found..
			if( foundCol != -1 ){
				(*foundAtLine) = tmpline;
				(*foundAtCol) = foundCol;
				(*matchLen) = text.length();
				return true;
			}
			//case of not found..
			tmpline++;
		}

		return false;

	}else{
		while( tmpline != 0 ){
			oneline = textLine( tmpline );
			kdDebug()<<oneline<<endl;
			if( firstTestedLine ){
				int index = (int)startCol;
				foundCol = oneline.findRev( text, index, casesensitive );
				firstTestedLine = false;
			}else{
				foundCol = oneline.findRev( text, -1, casesensitive );
			}

			// case of found..
			if( foundCol != -1 ){
				(*foundAtLine) = tmpline;
				(*foundAtCol) = foundCol;
				(*matchLen) = text.length();
				return true;
			}
			tmpline--;
		}
		return false;
	}
}

bool QEditorPart::searchText (unsigned int startLine, unsigned int startCol,
			const QRegExp &regexp, unsigned int *foundAtLine,
			unsigned int *foundAtCol, unsigned int *matchLen, bool backwards )
{
    unsigned int line = startLine;
    while( line<=numLines() ){
        QString str = textLine( line );
        int pos = -1;
        if( backwards ){
            pos = regexp.searchRev( str, line == startLine ? startCol : str.length() );
        } else {
            pos = regexp.search( str, line == startLine ? startCol : 0 );
        }

        if( pos != -1 ){
	    *foundAtLine = line;
            *foundAtCol = pos;
            *matchLen = regexp.matchedLength();
            return true;
        }

        if( backwards ){
	    --line;
        } else {
	    ++line;
        }
    }
    return false;
}

uint QEditorPart::mark (uint line)
{
    QTextDocument* textDoc = m_currentView->editor()->document();
    QTextParagraph* parag = textDoc->paragAt( line );
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
    QTextDocument* textDoc = m_currentView->editor()->document();
    QTextParagraph* parag = textDoc->paragAt( line );
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
    QTextDocument* textDoc = m_currentView->editor()->document();
    QTextParagraph* parag = textDoc->paragAt( line );
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
    QTextDocument* textDoc = m_currentView->editor()->document();
    QTextParagraph* parag = textDoc->paragAt( line );
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
    //m_marks.clear();

    QPtrList<KTextEditor::Mark> marks;
    marks.setAutoDelete( true );

    QTextDocument* textDoc = m_currentView->editor()->document();
    QTextParagraph* p = textDoc->firstParagraph();
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
    QTextDocument* textDoc = m_currentView->editor()->document();
    QTextParagraph* p = textDoc->firstParagraph();
    while( p ){
        ParagData* data = (ParagData*) p->extraData();
        if( data ){
            data->setMark( 0 );
        }
        p = p->next();
    }
}

void QEditorPart::setPixmap(KTextEditor::MarkInterface::MarkTypes mt, const QPixmap & pm)
{
    m_currentView->markerWidget()->setPixmap(mt, pm);
}

void QEditorPart::setDescription(KTextEditor::MarkInterface::MarkTypes mt, const QString & s)
{
    m_currentView->markerWidget()->setDescription(mt, s);
}

void QEditorPart::setMarksUserChangable(uint markMask)
{
    m_currentView->markerWidget()->setMarksUserChangable(markMask);
}

// -- ConfigInterface ---------------------------------------------------------------------

void QEditorPart::readConfig()
{
    QEditorSettings::self()->readConfig();
    m_currentView->configChanged();
}

void QEditorPart::writeConfig()
{
    kdDebug(9032) << "QEditorPart::writeConfig() - not implemented yet!" << endl;
}

void QEditorPart::readConfig(KConfig *)
{
    kdDebug(9032) << "QEditorPart::readConfig(KConfig *) - not implemented yet!" << endl;
}

void QEditorPart::writeConfig(KConfig *)
{
    kdDebug(9032) << "QEditorPart::writeConfig(KConfig *) - not implemented yet!" << endl;
}

void QEditorPart::readSessionConfig(KConfig *)
{
    kdDebug(9032) << "QEditorPart::readSessionConfig(KConfig *) - not implemented yet!" << endl;
}

void QEditorPart::writeSessionConfig(KConfig *)
{
    kdDebug(9032) << "QEditorPart::writeSessionConfig(KConfig *) - not implemented yet!" << endl;
}

void QEditorPart::configDialog()
{
    KDialogBase dlg(KDialogBase::Tabbed, i18n("QEditor Options"),
                    KDialogBase::Ok|KDialogBase::Cancel,
                    KDialogBase::Ok, 0,
                    "qeditor options dialog");

    GeneralConfigPage* generalPage = new GeneralConfigPage( dlg.addVBoxPage(i18n("General")) );
    generalPage->setPart( this );
    connect( &dlg, SIGNAL(okClicked()), generalPage, SLOT(accept()) );


    if( colorizer() ){
        HighlightingConfigPage* hlPage = new HighlightingConfigPage( dlg.addVBoxPage(i18n("Highlighting")) );
        hlPage->setEditor( this );
        connect( &dlg, SIGNAL(okClicked()), hlPage, SLOT(accept()) );
    }

    if( indenter() ){
        (void) indenter()->createConfigPage( this, &dlg );
    }

    emit configWidget( &dlg );

    if ( dlg.exec() )
    {
        QEditorSettings::self()->config()->sync();
        m_currentView->configChanged();
/*
        QPtrList<QEditorView> views = QEditorPartFactory::views();
        QPtrListIterator<QEditorView> it( views );
        while( it.current() )
        {
            it.current()->configChanged();
            ++it;
        }
*/
    }
}

// ----------------------------------------------------------------------------------

QSourceColorizer* QEditorPart::colorizer() const
{
    return m_currentView->editor()->colorizer();
}

QEditorIndenter* QEditorPart::indenter() const
{
    return m_currentView->editor()->indenter();
}

