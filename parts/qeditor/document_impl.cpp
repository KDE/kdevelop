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


using namespace KEditor;


DocumentImpl::DocumentImpl(Editor *parent, QWidget *parentWidget)
  : Document(parent), _widget(0)
{
  setInstance(QEditorPartFactory::instance());

  // create the editor
  _widget = new QEditor(parentWidget);
  _widget->setFont(QFont("courier", 12));
  setWidget(_widget);

  // create interfaces
  new ClipboardIfaceImpl(_widget, this, parent);
  new UndoIfaceImpl(_widget, this, parent);
  new CursorIfaceImpl(_widget, this, parent);
  new EditIfaceImpl(_widget, this, parent);
  new CodeCompletionDocumentIfaceImpl(_widget, this, parent );

  setXMLFile("qeditor_part.rc", true);
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
