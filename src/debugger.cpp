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
  kdDebug() << "setBreakpoint:" << fileName << endl;
  KParts::Part *part = PartController::getInstance()->partForURL(KURL(fileName));
  if( !part )
    return;
  MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
  if (!iface)
    return;
  
  // Temporarily disconnect so we don't get confused by receiving extra markChanged signals
  // This wouldn't be a problem if the debugging interfaces had explicit add/remove methods
  // rather than just toggle
  disconnect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
              this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
  iface->removeMark( lineNum, Breakpoint | ActiveBreakpoint | ReachedBreakpoint | DisabledBreakpoint );
  if( id != -1 ) {
    uint markType = Breakpoint;
    if( !pending )
      markType |= ActiveBreakpoint;
    if( !enabled )
      markType |= DisabledBreakpoint;
    iface->addMark( lineNum, markType );
  }
  connect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
           this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
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


void Debugger::markChanged( Mark mark, MarkInterfaceExtension::MarkChangeAction action )
{
#if (KDE_VERSION > 305)
  if( !sender()->inherits("KTextEditor::Document") )
    return;
  KTextEditor::Document* doc = (KTextEditor::Document*) sender();
  MarkInterfaceExtension* iface = KTextEditor::markInterfaceExtension( doc );
  if( !iface )
    return;
  if( mark.type & Breakpoint ) {
    if( !PartController::getInstance()->partForURL( doc->url() ) )
      return; // Probably means the document is being closed.
    switch( action ) {
    // Would be better to call distinct methods here rather than toggle...
    case MarkInterfaceExtension::MarkAdded:
    case MarkInterfaceExtension::MarkRemoved:
      emit toggledBreakpoint( doc->url().path(), mark.line );
      break;
    }
  }
#endif
}


void Debugger::partAdded( KParts::Part* part )
{
#if (KDE_VERSION > 305)
  MarkInterfaceExtension *iface = dynamic_cast<MarkInterfaceExtension*>(part);
  if( !iface )
    return;
  
  iface->setDescription((MarkInterface::MarkTypes)Breakpoint, i18n("Breakpoint"));
  iface->setPixmap((MarkInterface::MarkTypes)Breakpoint, *inactiveBreakpointPixmap());
  iface->setPixmap((MarkInterface::MarkTypes)ActiveBreakpoint, *activeBreakpointPixmap());
  iface->setPixmap((MarkInterface::MarkTypes)ReachedBreakpoint, *reachedBreakpointPixmap());
  iface->setPixmap((MarkInterface::MarkTypes)DisabledBreakpoint, *disabledBreakpointPixmap());
  iface->setPixmap((MarkInterface::MarkTypes)ExecutionPoint, *executionPointPixmap());
  iface->setMarksUserChangable( Bookmark | Breakpoint );
  
  connect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
           this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
#endif
}

#include "debugger.moc"
