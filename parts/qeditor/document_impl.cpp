#include <qeditor.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>


#include <kparts/partmanager.h>


#include "document_impl.h"
#include "qeditor_part.h"
#include "qeditor_factory.h"
#include "clipboard_iface_impl.h"
#include "undo_iface_impl.h"
#include "cursor_iface_impl.h"
#include "edit_iface_impl.h"
#include "codecompletion_iface_impl.h"

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kinstance.h>
#include <kfontdialog.h>

using namespace KEditor;


DocumentImpl::DocumentImpl(Editor *parent, QWidget *parentWidget)
  : Document(parent), _widget(0)
{
  setInstance(QEditorPartFactory::instance());

    KConfig* config = QEditorPartFactory::instance( )->config( );
    config->setGroup( "Font" );
    m_font.setFamily( config->readEntry( "Family" ) );
    m_font.setPointSize( config->readEntry( "Size" ).toInt( ) );

  // create the editor
  _widget = new QEditor(parentWidget);
  _widget->setFont( m_font );
  setWidget(_widget);

  // create interfaces
  new ClipboardIfaceImpl(_widget, this, parent);
  new UndoIfaceImpl(_widget, this, parent);
  new CursorIfaceImpl(_widget, this, parent);
  new EditIfaceImpl(_widget, this, parent);
  new CodeCompletionDocumentIfaceImpl(_widget, this, parent );

  setXMLFile("qeditor_part.rc", true);
  
  KAction* action;
  action = new KAction( i18n( "Font settings..." ), 0,
			this, SLOT( slotFontSettings( ) ),
			actionCollection( ), "settings_font" );
}

void
DocumentImpl::slotFontSettings( )
{
    KFontDialog* d = new KFontDialog( );
    d->setFont( _widget->font( ) );

    if( d->exec( ) == KFontDialog::Accepted ){
	m_font = d->font( );
	_widget->setFont( m_font );
	emit _widget->refresh( );

	KConfig* config = QEditorPartFactory::instance( )->config( );
	config->setGroup  ( "Font" );
	config->writeEntry( "Family", m_font.family( ) );
	config->writeEntry( "Size"  , m_font.pointSize( ) );
    }
}

bool DocumentImpl::openFile()
{
    if( ! _widget->openFile( m_file ) ){
        return FALSE;
    }

    resetModifiedTime();
    emit KEditor::Document::loaded(this);

    return TRUE;
}


bool DocumentImpl::saveFile()
{
    if (!shouldBeSaved())
        return false;

    if( !_widget->saveFile( m_file ) ){
        return false;
    }

    resetModifiedTime();
    emit KEditor::Document::saved(this);

    return true;
}


#include "document_impl.moc"
