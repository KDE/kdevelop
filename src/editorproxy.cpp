#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtimer.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <klocale.h>

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>
#if (KDE_VERSION > 304)
#include <ktexteditor/markinterfaceextension.h>
#endif


#include "partcontroller.h"
#include "core.h"


#include "editorproxy.h"


using namespace KTextEditor;

EditorProxy *EditorProxy::s_instance = 0;


EditorProxy::EditorProxy()
  : QObject()
{
  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(activePartChanged(KParts::Part*)));
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

  ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>(part->widget());
  if (iface)
    iface->setCursorPosition(lineNum, 0);
}


void EditorProxy::clearExecutionPoint()
{
  QPtrListIterator<KParts::Part> it(*PartController::getInstance()->parts());
  for ( ; it.current(); ++it)
  {
    MarkInterface *iface = dynamic_cast<MarkInterface*>(it.current());
    if (!iface)
      continue;

    QPtrList<Mark> list = iface->marks();
    QPtrListIterator<Mark> markIt(list);
    for( ; markIt.current(); ++markIt ) {
      Mark* mark = markIt.current();
      if( mark->type & ExecutionPoint )
        iface->removeMark( mark->line, ExecutionPoint );
    }
  }
}


void EditorProxy::setExecutionPoint(KParts::Part *part, int lineNum)
{
  clearExecutionPoint();

  MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
  if (iface)
    iface->addMark( lineNum, ExecutionPoint );
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

  ViewCursorInterface *cursorIface = dynamic_cast<ViewCursorInterface*>(ro_part->widget());
  EditInterface *editIface = dynamic_cast<EditInterface*>(ro_part);

  if (!cursorIface || !editIface || !ro_part)
  {
    Core::getInstance()->fillContextMenu(popup, 0);
  }
  else
  {
    uint line, col;
    cursorIface->cursorPosition(&line, &col);
    QString linestr = editIface->textLine(line);
    int startPos = QMAX(QMIN((int)col, (int)linestr.length()-1), 0);
    int endPos = startPos;
    while (startPos >= 0 && linestr[startPos].isLetter())
        startPos--;
    while (endPos < (int)linestr.length() && linestr[endPos].isLetter())
        endPos++;
    QString wordstr = (startPos==endPos)?
        QString() : linestr.mid(startPos+1, endPos-startPos-1);
    kdDebug(9000) << "Word:" << wordstr << ":" << endl;
    EditorContext context(ro_part->url(), line, col,
                          linestr, wordstr);
    Core::getInstance()->fillContextMenu(popup, &context);
  }
}


void EditorProxy::removeBreakpoint(KParts::Part *part, int lineNum)
{
  MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
  if (!iface)
    return;
  iface->removeMark( lineNum, ActiveBreakpoint | ReachedBreakpoint | InactiveBreakpoint );
}


void EditorProxy::setBreakpoint(KParts::Part *part, int lineNum, bool enabled, bool pending)
{
  MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
  if (!iface)
    return;

  removeBreakpoint(part, lineNum);

  uint markType = InactiveBreakpoint;
  if (enabled && pending)
    markType = ActiveBreakpoint;
  else if (enabled && !pending)
    markType = ReachedBreakpoint;
 
  iface->addMark( lineNum, markType );
}


void EditorProxy::activePartChanged(KParts::Part *part)
{
#if (KDE_VERSION > 304)
  MarkInterfaceExtension *iface = dynamic_cast<MarkInterfaceExtension*>(part);
  if (iface)
  {
    iface->setDescription((MarkInterface::MarkTypes)Bookmark, i18n("Bookmark"));
    iface->setDescription((MarkInterface::MarkTypes)ActiveBreakpoint, i18n("Active Breakpoint"));
    iface->setDescription((MarkInterface::MarkTypes)ReachedBreakpoint, i18n("Breakpoint Reached"));
    iface->setDescription((MarkInterface::MarkTypes)InactiveBreakpoint, i18n("Inactive Breakpoint"));

    iface->setMarksUserChangable( Bookmark |
                                  ActiveBreakpoint |
                                  ReachedBreakpoint |
                                  InactiveBreakpoint );
  }
#endif
}


#include "editorproxy.moc"
