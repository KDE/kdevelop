#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtimer.h>

#include <kdeversion.h>
#include <kdebug.h>

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>

#include "partcontroller.h"
#include "core.h"
#include "debugger.h"


#include "editorproxy.h"


using namespace KTextEditor;

EditorProxy *EditorProxy::s_instance = 0;


EditorProxy::EditorProxy()
  : QObject()
{
}


EditorProxy *EditorProxy::getInstance()
{
  if (!s_instance)
    s_instance = new EditorProxy;

  return s_instance;
}


void EditorProxy::setLineNumber(KParts::Part *part, int lineNum)
{
  if (!part || !part->inherits("KTextEditor::Document"))
    return;

  if ( lineNum < 0 )
    return;

  ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>(part->widget());
  if (iface)
    iface->setCursorPosition(lineNum, 0);
}


void EditorProxy::installPopup(KParts::Part *part, QPopupMenu *popup)
{
  if (part->inherits("KTextEditor::Document") && part->widget())
  {
    PopupMenuInterface *iface = dynamic_cast<PopupMenuInterface*>(part->widget());
    if (iface)
    {
      iface->installPopup(popup);

      connect(popup, SIGNAL(aboutToShow()), this, SLOT(popupAboutToShow()));
    }
  }
}


void EditorProxy::popupAboutToShow()
{
  QPopupMenu *popup = (QPopupMenu*)sender();
  if (!popup)
    return;

  // ugly hack: remove all but the "original" items
  for (int index=popup->count()-1; index >= 0; --index)
  {
    int id = popup->idAt(index);
    if (m_popupIds.contains(id) == 0)
    {
      QMenuItem *item = popup->findItem(id);
      if (item->popup())
	delete item->popup();
      else
          popup->removeItemAt(index);
      kdDebug(9000) << "removed id " << id << " at index " << index << endl;
    } else {
        kdDebug(9000) << "leaving id " << id << endl;
    }
  }

  // ugly hack: mark the "original" items 
  m_popupIds.resize(popup->count());
  for (uint index=0; index < popup->count(); ++index)
    m_popupIds[index] = popup->idAt(index);
  
  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(PartController::getInstance()->activePart());
  if (!ro_part)
    return;

  // fill the menu in the file context
  FileContext context(ro_part->url().path(), false);
  Core::getInstance()->fillContextMenu(popup, &context);
  
  // fill the menu in the editor context
  if (!ro_part->widget())
    return;

  SelectionInterface *selectIface = dynamic_cast<SelectionInterface*>(ro_part);
  ViewCursorInterface *cursorIface = dynamic_cast<ViewCursorInterface*>(ro_part->widget());
  EditInterface *editIface = dynamic_cast<EditInterface*>(ro_part);

  QString wordstr, linestr;
  uint line, col;
  line = col = 0;
  if( selectIface && selectIface->hasSelection() )
  {
    wordstr = selectIface->selection();
    // Why doesn't SelectionInterface have a way to get the bounds?
  }
  if( cursorIface && editIface )
  {
    cursorIface->cursorPosition(&line, &col);
    linestr = editIface->textLine(line);
    if( wordstr.isEmpty() ) {
      int startPos = QMAX(QMIN((int)col, (int)linestr.length()-1), 0);
      int endPos = startPos;
      while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' ) )
          startPos--;
      while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
          endPos++;
      wordstr = (startPos==endPos)?
          QString() : linestr.mid(startPos+1, endPos-startPos-1);  
    }
    kdDebug(9000) << "Word:" << wordstr << ":" << endl;
    EditorContext context(ro_part->url(), line, col, linestr, wordstr);
    Core::getInstance()->fillContextMenu(popup, &context);
  } else {
    Core::getInstance()->fillContextMenu(popup, 0);
  }
  
  // Remove redundant separators (any that are first, last, or doubled)
  bool lastWasSeparator = true;
  for( uint i = 0; i < popup->count(); ) {
    int id = popup->idAt( i );
    if( lastWasSeparator && popup->findItem( id )->isSeparator() ) {
      popup->removeItem( id );
      // Since we removed an item, don't increment i
    } else {
      lastWasSeparator = false;
      i++;
    }
  }
  if( lastWasSeparator && popup->count() > 0 )
    popup->removeItem( popup->idAt( popup->count() - 1 ) );
}

#include "editorproxy.moc"
