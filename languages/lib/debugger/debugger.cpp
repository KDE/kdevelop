
#include "debugger.h"

#include <kdebug.h>
#include <klocale.h>
#include <ktexteditor/document.h>

// #include "editorproxy.h"
#include <kdevpartcontroller.h>


using namespace KTextEditor;

Debugger *Debugger::s_instance = 0;

Debugger::Debugger(KDevPartController *partController)
    :m_partController(partController)
{
    connect( m_partController, SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(partAdded(KParts::Part*)) );
}


Debugger::~Debugger()
{}


// Debugger *Debugger::getInstance()
// {
//     if (!s_instance)
//         s_instance = new Debugger;
//
//     return s_instance;
// }


void Debugger::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
    KParts::Part *part = m_partController->partForURL(KURL(fileName));
    if( !part )
        return;

    MarkInterface *iface = dynamic_cast<MarkInterface*>(part);
    if (!iface)
        return;

    // Temporarily disconnect so we don't get confused by receiving extra
    // marksChanged signals
    disconnect( part, SIGNAL(marksChanged()), this, SLOT(marksChanged()) );
    iface->removeMark( lineNum, Breakpoint | ActiveBreakpoint | ReachedBreakpoint | DisabledBreakpoint );

    BPItem bpItem(fileName, lineNum);
    QValueList<BPItem>::Iterator it = BPList.find(bpItem);
    if (it != BPList.end())
    {
//        kdDebug(9012) << "Removing BP=" << fileName << ":" << lineNum << endl;
        BPList.remove(it);
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
//        kdDebug(9012) << "Appending BP=" << fileName << ":" << lineNum << endl;
        BPList.append(BPItem(fileName, lineNum));
    }

    connect( part, SIGNAL(marksChanged()), this, SLOT(marksChanged()) );
}


void Debugger::clearExecutionPoint()
{
    QPtrListIterator<KParts::Part> it(*m_partController->parts());
    for ( ; it.current(); ++it)
    {
        MarkInterface *iface = dynamic_cast<MarkInterface*>(it.current());
        if (!iface)
            continue;

        QPtrList<Mark> list = iface->marks();
        QPtrListIterator<Mark> markIt(list);
        for( ; markIt.current(); ++markIt )
        {
            Mark* mark = markIt.current();
            if( mark->type & ExecutionPoint )
                iface->removeMark( mark->line, ExecutionPoint );
        }
    }
}


void Debugger::gotoExecutionPoint(const KURL &url, int lineNum)
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
        MarkInterface* iface = KTextEditor::markInterface( doc );

        if (iface)
        {
            if( !m_partController->partForURL( doc->url() ) )
                return; // Probably means the document is being closed.

            KTextEditor::Mark *m;
            QValueList<BPItem> oldBPList = BPList;
            QPtrList<KTextEditor::Mark> newMarks = iface->marks();

            // Compare the oldBPlist to the new list from the editor.
            //
            // If we don't have some of the old breakpoints in the new list
            // then they have been moved by the user adding or removing source
            // code. Remove these old breakpoints
            //
            // If we _can_ find these old breakpoints in the newlist then
            // nothing has happened to them. We can just ignore these and to
            // do that we must remove them from the new list.

            bool bpchanged = false;

            for (uint i = 0; i < oldBPList.count(); i++)
            {
                if (oldBPList[i].fileName() != doc->url().path())
                    continue;

                bool found=false;
                for (uint newIdx=0; newIdx < newMarks.count(); newIdx++)
                {
                    m = newMarks.at(newIdx);
                    if ((m->type & Breakpoint) &&
                            m->line == oldBPList[i].lineNum() &&
                            doc->url().path() == oldBPList[i].fileName())
                    {
                        newMarks.remove(newIdx);
                        found=true;
                        break;
                    }
                }

                if (!found)
                {
                    emit toggledBreakpoint( doc->url().path(), oldBPList[i].lineNum() );
                    bpchanged = true;
                }
            }

            // Any breakpoints left in the new list are the _new_ position of
            // the moved breakpoints. So add these as new breakpoints via
            // toggling them.
            for (uint i = 0; i < newMarks.count(); i++)
            {
                m = newMarks.at(i);
                if (m->type & Breakpoint)
                {
                    emit toggledBreakpoint( doc->url().path(), m->line );
                    bpchanged = true;
                }
            }

            if ( bpchanged )
            {
                //bring focus back to the editor
                m_partController->activatePart( doc );
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

    connect( part, SIGNAL(marksChanged()), this, SLOT(marksChanged()) );
}

#include "debugger.moc"
