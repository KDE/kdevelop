#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtimer.h>


#include <kdebug.h>

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>


#include "partcontroller.h"
#include "core.h"


#include "editorproxy.h"


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

  KTextEditor::ViewCursorInterface *iface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
  if (iface)
    iface->setCursorPosition(lineNum, 0);
}


void EditorProxy::clearExecutionPoint()
{
  QPtrListIterator<KParts::Part> it(*PartController::getInstance()->parts());
  for ( ; it.current(); ++it)
  {
    KTextEditor::MarkInterface *iface = dynamic_cast<KTextEditor::MarkInterface*>(it.current());
    if (!iface)
      continue;

    for (KTextEditor::Mark *mark = iface->marks().first(); mark != 0; mark = iface->marks().next())
      if (mark->type == KTextEditor::MarkInterface::markType05)
	iface->removeMark(mark->line, mark->type);
  }
}


void EditorProxy::setExecutionPoint(KParts::Part *part, int lineNum)
{
  clearExecutionPoint();

  KTextEditor::MarkInterface *iface = dynamic_cast<KTextEditor::MarkInterface*>(part);
  if (iface)
    iface->setMark(lineNum, KTextEditor::MarkInterface::markType05);
}


void EditorProxy::installPopup(KParts::Part *part, QPopupMenu *popup)
{
  if (part->inherits("KTextEditor::Document") && part->widget())
  {
    KTextEditor::PopupMenuInterface *iface = dynamic_cast<KTextEditor::PopupMenuInterface*>(part->widget());
    if (iface)
    {
      iface->installPopup(popup);

      connect(popup, SIGNAL(aboutToShow()), this, SLOT(popupAboutToShow()));
      connect(popup, SIGNAL(aboutToHide()), this, SLOT(popupAboutToHide()));
    }
  }
}


void EditorProxy::popupAboutToShow()
{
  QPopupMenu *popup = (QPopupMenu*)sender();
  if (!popup)
    return;

  // ugly hack: mark the "original" items 
  m_popupIds.resize(popup->count());
  for (uint index=0; index < popup->count(); ++index)
    m_popupIds[index] = popup->idAt(index);
  
  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(PartController::getInstance()->activePart());
  if (!ro_part)
    return;

  // fill the menu in the file context
  FileContext context(ro_part->url().path());
  Core::getInstance()->fillContextMenu(popup, &context);
  
  // fill the menu in the editor context
  if (!ro_part->widget())
    return;

  KTextEditor::ViewCursorInterface *cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>(ro_part->widget());
  KTextEditor::EditInterface *editIface = dynamic_cast<KTextEditor::EditInterface*>(ro_part);

  if (!cursorIface || !editIface || !ro_part)
  {
    Core::getInstance()->fillContextMenu(popup, 0);
  }
  else
  {
    uint line, col;
    cursorIface->cursorPosition(&line, &col);
    EditorContext context(ro_part->url(), line, editIface->textLine(line), col);
    Core::getInstance()->fillContextMenu(popup, &context);
  }
}


void EditorProxy::popupAboutToHide()
{
  m_popup = (QPopupMenu*)sender();

  QTimer::singleShot(0, this, SLOT(slotDeletePopup()));
}


void EditorProxy::deletePopup()
{
  if (!m_popup)
    return;

  // ugly hack: remove all but the "original" items
  for (int index=m_popup->count()-1; index >= 0; --index)
  {
    int id = m_popup->idAt(index);
    if (m_popupIds.contains(id) == 0)
    {
      QMenuItem *item = m_popup->findItem(id);
      if (item->popup())
	delete item->popup();
      m_popup->removeItemAt(index);
    }
  }

  m_popup = 0;
}


#include "editorproxy.moc"
