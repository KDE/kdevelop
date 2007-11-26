
#include "breakpointcontroller.h"

#include <QPixmap>

#include <kdebug.h>
#include <klocale.h>
#include <ktexteditor/document.h>

// #include "editorproxy.h"
#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

using namespace KTextEditor;

BreakpointController::BreakpointController(QObject* parent)
    : QObject(parent)
{
    connect( KDevelop::ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)),
             this, SLOT(documentLoaded(KDevelop::IDocument*)) );
}


BreakpointController::~BreakpointController()
{}


void BreakpointController::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
    KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->activeDocument();
    if (!document)
      return;

    MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument());
    if (!iface)
        return;

    // Temporarily disconnect so we don't get confused by receiving extra
    // marksChanged signals
    disconnect( document->textDocument(), SIGNAL(marksChanged()), this, SLOT(marksChanged()) );
    iface->removeMark( lineNum, Breakpoint | ActiveBreakpoint | ReachedBreakpoint | DisabledBreakpoint );

    BPItem bpItem(fileName, lineNum);
    QList<BPItem>::Iterator it = BPList.find(bpItem);
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

    connect( document->textDocument(), SIGNAL(marksChanged()), this, SLOT(marksChanged()) );
}


void BreakpointController::clearExecutionPoint()
{
    foreach (KDevelop::IDocument* document, KDevelop::ICore::self()->documentController()->openDocuments())
    {
        MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument());
        if (!iface)
            continue;

        QHashIterator<int, KTextEditor::Mark*> it = iface->marks();
        while (it.hasNext())
        {
            Mark* mark = it.next().value();
            if( mark->type & ExecutionPoint )
                iface->removeMark( mark->line, ExecutionPoint );
        }
    }
}


void BreakpointController::gotoExecutionPoint(const KUrl &url, int lineNum)
{
    clearExecutionPoint();

    KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->openDocument(url, KTextEditor::Cursor(lineNum, 0));

    if( !document )
        return;

    MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument());
    if( !iface )
        return;

    iface->addMark( lineNum, ExecutionPoint );
}

void BreakpointController::marksChanged()
{
    if(KTextEditor::Document* doc = qobject_cast<KTextEditor::Document*>(sender()))
    {
        MarkInterface* iface = dynamic_cast<KTextEditor::MarkInterface*>( doc );

        if (iface)
        {
            KTextEditor::Mark *m;
            QList<BPItem> oldBPList = BPList;
            QHashIterator<int, KTextEditor::Mark*> newMarks = iface->marks();

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

            for (int i = 0; i < oldBPList.count(); i++)
            {
                if (oldBPList[i].fileName() != doc->url().path())
                    continue;

                bool found=false;

                newMarks.toFront();
                while (newMarks.hasNext())
                {
                    m = newMarks.next().value();
                    if ((m->type & Breakpoint) &&
                            m->line == oldBPList[i].lineNum() &&
                            doc->url().path() == oldBPList[i].fileName())
                    {
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
            newMarks.toFront();
            while (newMarks.hasNext())
            {
                m = newMarks.next().value();
                if (m->type & Breakpoint)
                {
                    emit toggledBreakpoint( doc->url().path(), m->line );
                    bpchanged = true;
                }
            }

            if ( bpchanged && KDevelop::ICore::self()->documentController()->activeDocument() && KDevelop::ICore::self()->documentController()->activeDocument()->textDocument() == doc )
            {
                //bring focus back to the editor
                // TODO probably want a different command here
                KDevelop::ICore::self()->documentController()->activateDocument(KDevelop::ICore::self()->documentController()->activeDocument());
            }
        }
    }
}


void BreakpointController::documentLoaded(KDevelop::IDocument* document)
{
    MarkInterface *iface = dynamic_cast<MarkInterface*>(document->textDocument());
    if( !iface )
        return;

    iface->setMarkDescription((MarkInterface::MarkTypes)Breakpoint, i18n("Breakpoint"));
    iface->setMarkPixmap((MarkInterface::MarkTypes)Breakpoint, *inactiveBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)ActiveBreakpoint, *activeBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)ReachedBreakpoint, *reachedBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)DisabledBreakpoint, *disabledBreakpointPixmap());
    iface->setMarkPixmap((MarkInterface::MarkTypes)ExecutionPoint, *executionPointPixmap());
    iface->setEditableMarks( Bookmark | Breakpoint );

    connect( document->textDocument(), SIGNAL(marksChanged()), this, SLOT(marksChanged()) );
}

const QPixmap* BreakpointController::inactiveBreakpointPixmap()
{
  const char*breakpoint_gr_xpm[]={
  "11 16 6 1",
  "c c #c6c6c6",
  "d c #2c2c2c",
  "# c #000000",
  ". c None",
  "a c #ffffff",
  "b c #555555",
  "...........",
  "...........",
  "...#####...",
  "..#aaaaa#..",
  ".#abbbbbb#.",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  ".#bbbbbbb#.",
  "..#bdbdb#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_gr_xpm );
  return &pixmap;
}

const QPixmap* BreakpointController::activeBreakpointPixmap()
{
  const char* breakpoint_xpm[]={
  "11 16 6 1",
  "c c #c6c6c6",
  ". c None",
  "# c #000000",
  "d c #840000",
  "a c #ffffff",
  "b c #ff0000",
  "...........",
  "...........",
  "...#####...",
  "..#aaaaa#..",
  ".#abbbbbb#.",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  ".#bbbbbbb#.",
  "..#bdbdb#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_xpm );
  return &pixmap;
}

const QPixmap* BreakpointController::reachedBreakpointPixmap()
{
  const char*breakpoint_bl_xpm[]={
  "11 16 7 1",
  "a c #c0c0ff",
  "# c #000000",
  "c c #0000c0",
  "e c #0000ff",
  "b c #dcdcdc",
  "d c #ffffff",
  ". c None",
  "...........",
  "...........",
  "...#####...",
  "..#ababa#..",
  ".#bcccccc#.",
  "#acccccccc#",
  "#bcadadace#",
  "#acccccccc#",
  "#bcadadace#",
  "#acccccccc#",
  ".#ccccccc#.",
  "..#cecec#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_bl_xpm );
  return &pixmap;
}

const QPixmap* BreakpointController::disabledBreakpointPixmap()
{
  const char*breakpoint_wh_xpm[]={
  "11 16 7 1",
  "a c #c0c0ff",
  "# c #000000",
  "c c #0000c0",
  "e c #0000ff",
  "b c #dcdcdc",
  "d c #ffffff",
  ". c None",
  "...........",
  "...........",
  "...#####...",
  "..#ddddd#..",
  ".#ddddddd#.",
  "#ddddddddd#",
  "#ddddddddd#",
  "#ddddddddd#",
  "#ddddddddd#",
  "#ddddddddd#",
  ".#ddddddd#.",
  "..#ddddd#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_wh_xpm );
  return &pixmap;
}

const QPixmap* BreakpointController::executionPointPixmap()
{
  const char*exec_xpm[]={
  "11 16 4 1",
  "a c #00ff00",
  "b c #000000",
  ". c None",
  "# c #00c000",
  "...........",
  "...........",
  "...........",
  "#a.........",
  "#aaa.......",
  "#aaaaa.....",
  "#aaaaaaa...",
  "#aaaaaaaaa.",
  "#aaaaaaa#b.",
  "#aaaaa#b...",
  "#aaa#b.....",
  "#a#b.......",
  "#b.........",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( exec_xpm );
  return &pixmap;
}

#include "breakpointcontroller.moc"
