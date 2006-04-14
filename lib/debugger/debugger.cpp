
#include "debugger.h"

#include <kdebug.h>
#include <klocale.h>
#include <ktexteditor/document.h>

#include <kdevdocumentcontroller.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>


using namespace KTextEditor;

Debugger *Debugger::s_instance = 0;

Debugger::Debugger(KDevDocumentController *partController)
    :m_partController(partController)
{
    connect( m_partController, SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(partAdded(KParts::Part*)) );
}


Debugger::~Debugger()
{}


void Debugger::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
    KParts::Part *part = m_partController->partForURL(KUrl(fileName));
    if( !part )
        return;

    MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
    if (!iface)
        return;

    // Temporarily disconnect so we don't get confused by receiving extra
    // marksChanged signals
    disconnect( part, SIGNAL(marksChanged(KTextEditor::Document*)), this, SLOT(marksChanged()) );
    iface->removeMark( lineNum, Breakpoint | ActiveBreakpoint | ReachedBreakpoint | DisabledBreakpoint );

    BPItem bpItem(fileName, lineNum);
    int i = BPList.indexOf(bpItem);
    if (i != -1)
    {
//        kDebug(9012) << "Removing BP=" << fileName << ":" << lineNum << endl;
        BPList.removeAt(i);
    }

    // An id of -1 means this breakpoint should be hidden from the user.
    // I believe this functionality is not used presently.
    if( id != -1 )
    {
        uint markType = Breakpoint;
        if( !pending )
            markType |= ActiveBreakpoint;
        if( !enabled )
            markType |= DisabledBreakpoint;
        iface->addMark( lineNum, markType );
//        kDebug(9012) << "Appending BP=" << fileName << ":" << lineNum << endl;
        BPList.append(BPItem(fileName, lineNum));
    }

    connect( part, SIGNAL(marksChanged(KTextEditor::Document*)), this, SLOT(marksChanged()) );
}


void Debugger::clearExecutionPoint()
{
    foreach  (KParts::Part* part, m_partController->parts())
    {
        MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
        if (!iface)
            continue;

        foreach (Mark* mark, iface->marks())
            if( mark->type & ExecutionPoint )
                iface->removeMark( mark->line, ExecutionPoint );
    }
}


void Debugger::gotoExecutionPoint(const KUrl &url, int lineNum)
{
    clearExecutionPoint();

    m_partController->editDocument(url, lineNum);

    KParts::Part *part = m_partController->partForURL(url);
    if( !part )
        return;
    MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
    if( !iface )
        return;

    iface->addMark( lineNum, ExecutionPoint );
}

void Debugger::marksChanged()
{
    if(sender()->inherits("KTextEditor::Document") )
    {
        KTextEditor::Document* doc = (KTextEditor::Document*) sender();
        KTextEditor::MarkInterface* iface = qobject_cast<KTextEditor::MarkInterface*>( doc );

        if (iface)
        {
            if( !m_partController->partForURL( doc->url() ) )
                return; // Probably means the document is being closed.

            Q3ValueList<BPItem> oldBPList = BPList;
            QHash<int, Mark*> newMarks = iface->marks();

            // Compare the oldBPlist to the new list from the editor.
            //
            // If we don't have some of the old breakpoints in the new list
            // then they have been moved by the user adding or removing source
            // code. Remove these old breakpoints
            //
            // If we _can_ find these old breakpoints in the newlist then
            // nothing has happened to them. We can just ignore these and to
            // do that we must remove them from the new list.

            for (int i = 0; i < oldBPList.count(); i++)
            {
                if (oldBPList[i].fileName() != doc->url().path())
                    continue;

                bool found=false;
                for (QHash<int, Mark*>::Iterator it = newMarks.begin(); it != newMarks.end(); ++it)
                {
                    Mark* m = it.value();
                    if ((m->type & Breakpoint) &&
                            m->line == oldBPList[i].lineNum() &&
                            doc->url().path() == oldBPList[i].fileName())
                    {
                        newMarks.erase(it);
                        found = true;
                        break;
                    }
                }

                if (!found)
                    emit toggledBreakpoint( doc->url().path(), oldBPList[i].lineNum() );
            }

            // Any breakpoints left in the new list are the _new_ position of
            // the moved breakpoints. So add these as new breakpoints via
            // toggling them.
            foreach (Mark* m, newMarks)
            {
                if (m->type & Breakpoint)
                    emit toggledBreakpoint( doc->url().path(), m->line );
            }
        }
    }
}


void Debugger::partAdded( KParts::Part* part )
{
    MarkInterface *iface = qobject_cast<MarkInterface*>(part);
    if( !iface )
        return;

    iface->setMarkDescription((MarkInterface::MarkTypes)Breakpoint, i18n("Breakpoint"));
    iface->setMarkPixmap((MarkInterface::MarkTypes)Breakpoint, *inactiveBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)ActiveBreakpoint, *activeBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)ReachedBreakpoint, *reachedBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)DisabledBreakpoint, *disabledBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)ExecutionPoint, *executionPointPixmap());
    iface->setMarksUserChangable( Bookmark | Breakpoint );

    connect( part, SIGNAL(marksChanged(KTextEditor::Document*)), this, SLOT(marksChanged()) );
}

#include "debugger.moc"
