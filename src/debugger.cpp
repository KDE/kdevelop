#include <kdebug.h>
#include <klocale.h>
#include <ktexteditor/document.h>

#include "editorproxy.h"
#include "partcontroller.h"

#include "debugger.h"

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
  // Temporarily disconnect so we don't get confused by receiving extra markChanged signals
  // This wouldn't be a problem if the debugging interfaces had explicit add/remove methods
  // rather than just toggle
//TODO: Remove if there is no problem with breakpoint marks
/*
  disconnect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
              this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
*/
  disconnect( part, SIGNAL(marksChanged()),
              this, SLOT(marksChanged()) );
  iface->removeMark( lineNum, Breakpoint | ActiveBreakpoint | ReachedBreakpoint | DisabledBreakpoint );
  if (BPList.contains(lineNum))
  {
    QValueList<int>::Iterator it;
    it = BPList.find(lineNum);
    BPList.remove(it);
  }

  if( id != -1 ) {
    uint markType = Breakpoint;
    if( !pending )
      markType |= ActiveBreakpoint;
    if( !enabled )
      markType |= DisabledBreakpoint;
    iface->addMark( lineNum, markType );
    BPList.append(lineNum);
  }
//TODO: Remove if there is no problem with breakpoint marks
/*
  connect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
           this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
*/
  connect( part, SIGNAL(marksChanged()),
           this, SLOT(marksChanged()) );
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


//TODO: This method should not be needed anymore, as the marksChanged() does the job. (Andras Mantia)
/*
void Debugger::markChanged( Mark mark, MarkInterfaceExtension::MarkChangeAction action )
{
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
}
*/

void Debugger::marksChanged()
{
 if(sender()->inherits("KTextEditor::Document") )
 {
    KTextEditor::Document* doc = (KTextEditor::Document*) sender();
    MarkInterface* iface = KTextEditor::markInterface( doc );
    if (iface)
    {
      if( !PartController::getInstance()->partForURL( doc->url() ) )
         return; // Probably means the document is being closed.
      KTextEditor::Mark *m;
      KTextEditor::Mark mark;
      QValueList<int> oldBPList = BPList;
      for (uint i = 0; i < oldBPList.count(); i++)
      {
        emit toggledBreakpoint( doc->url().path(), oldBPList[i] );
      }
      BPList.clear();
      QPtrList<KTextEditor::Mark> newMarks = iface->marks();
      for (uint i = 0; i < newMarks.count(); i++)
      {
        m = newMarks.at(i);
        if (m->type & Breakpoint)
        {
          emit toggledBreakpoint( doc->url().path(), m->line );
        }
       }
   }
 }
}


void Debugger::partAdded( KParts::Part* part )
{
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
//TODO: Remove if there is no problem with breakpoint marks
/*
  connect( part, SIGNAL(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)),
           this, SLOT(markChanged(KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction)) );
*/
  connect( part, SIGNAL(marksChanged()),
           this, SLOT(marksChanged()) );
}

#include "debugger.moc"
