#include "debugger.h"

#include <kdebug.h>
#include <klocale.h>
#include <ktexteditor/document.h>

#include "editorproxy.h"
#include "partcontroller.h"

using namespace KTextEditor;


Debugger *Debugger::s_instance = 0;

Debugger::Debugger()
{
  connect( PartController::getInstance(), SIGNAL(partAdded(KParts::Part*)),
           this, SLOT(partAdded(KParts::Part*)) );
}


Debugger::~Debugger()
{
}


Debugger *Debugger::getInstance()
{
  if (!s_instance)
    s_instance = new Debugger;

  return s_instance;
}


void Debugger::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
  KParts::Part *part = PartController::getInstance()->partForURL(KURL(fileName));
  if( !part )
    return;
  MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
  if (!iface)
    return;
  
  blockSignals(true); // To avoid markChanged() getting called
  iface->removeMark( lineNum, Breakpoint | ActiveBreakpoint | ReachedBreakpoint );
  if( id != -1 ) {
    uint markType = Breakpoint;
    if( enabled )
      markType |= ActiveBreakpoint;
    if( enabled && !pending )
      markType |= ReachedBreakpoint;
    iface->addMark( lineNum, markType );
  }
  blockSignals(false);
}


void Debugger::clearExecutionPoint()
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


void Debugger::gotoExecutionPoint(const KURL &url, int lineNum)
{
  clearExecutionPoint();
  
  PartController::getInstance()->editDocument(url, lineNum);
  
  KParts::Part *part = PartController::getInstance()->partForURL(url);
  if( !part )
    return;
  MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
  if( !iface )
    return;
  
  iface->addMark( lineNum, ExecutionPoint );
}


void Debugger::markChanged( KTextEditor::Mark mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction action )
{
#if (KDE_VERSION > 304)
  if( !sender()->inherits("KTextEditor::Document") )
    return;
  KTextEditor::Document* doc = (KTextEditor::Document*) sender();
  MarkInterfaceExtension* iface = KTextEditor::markInterfaceExtension( doc );
  if( !iface )
    return;
  if( mark.type & Breakpoint ) {
    switch( action ) {
    case MarkInterfaceExtension::MarkAdded:
//      emit Debugger::getInstance()->toggledBreakpoint( doc->url()->path(), mark.line );
//      break;
    case MarkInterfaceExtension::MarkRemoved:
      emit toggledBreakpoint( doc->url().path(), mark.line );
      break;
    }
  }
#endif
}


void Debugger::partAdded( KParts::Part* part )
{
#if (KDE_VERSION > 304)
  MarkInterfaceExtension *iface = dynamic_cast<MarkInterfaceExtension*>(part);
  if( !iface )
    return;
  
  iface->setDescription((MarkInterface::MarkTypes)Bookmark, i18n("Bookmark"));
  iface->setDescription((MarkInterface::MarkTypes)Breakpoint, i18n("Breakpoint"));
  iface->setMarksUserChangable( Bookmark | Breakpoint );
  
  connect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
           this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
#endif
}

#include "debugger.moc"
