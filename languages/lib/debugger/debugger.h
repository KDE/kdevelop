#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include <qvaluelist.h>

#include "kdevdebugger.h"

#include <kparts/part.h>
#include <ktexteditor/markinterface.h>

#include <kdeversion.h>
#if (KDE_VERSION > 305)
# include <ktexteditor/markinterfaceextension.h>
#else
# include "kde30x_markinterfaceextension.h"
#endif

class KDevPartController;

/**
* Describes a single breakpoint in the system
*
* This is used so that we can track the breakpoints and move them appropriately
* as the user adds or removes lines of code before breakpoints.
*/

class BPItem
{
public:
    /**
    * default ctor - required from QValueList
    */
    BPItem() : m_fileName(""), m_lineNum(0)
    {}

    BPItem( const QString& fileName, const uint lineNum)
            : m_fileName(fileName),
            m_lineNum(lineNum)
    {}

    uint lineNum() const        {  return m_lineNum; }
    QString fileName() const    {  return m_fileName; }

    bool operator==( const BPItem& rhs ) const
    {
        return (m_fileName == rhs.m_fileName
                && m_lineNum == rhs.m_lineNum);
    }

private:
    QString m_fileName;
    uint m_lineNum;
};


/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class Debugger : public KDevDebugger
{
    Q_OBJECT

public:

    /**
    */
//     static Debugger *getInstance();

    /**
    * Controls the breakpoint icon being displayed in the editor through the
    * markinterface
    *
    * @param fileName   The breakpoint is added or removed from this file
    * @param lineNum    ... at this line number
    * @param id         This is an internal id. which has a special number
    *                   that prevents us changing the mark icon. (why?)
    * @param enabled    The breakpoint could be enabled, disabled
    * @param pending    pending or active. Each state has a different icon.
    */
    void setBreakpoint(const QString &fileName, int lineNum,
                       int id, bool enabled, bool pending);

    /**
    * Displays an icon in the file at the line that the debugger has stoped
    * at.
    * @param url        The file the debugger has stopped at.
    * @param lineNum    The line number to display. Note: We may not know it.
    */
    void gotoExecutionPoint(const KURL &url, int lineNum=-1);

    /**
    * Remove the executution point being displayed.
    */
    void clearExecutionPoint();

// protected:

    Debugger(KDevPartController *partController);
    ~Debugger();

private slots:

    /**
    * Whenever a new part is added this slot gets triggered and we then
    * look for a MarkInterfaceExtension part. When it is a
    * MarkInterfaceExtension part we set the various pixmaps of the
    * breakpoint icons.
    */
    void partAdded( KParts::Part* part );

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void marksChanged();

private:
    enum MarkType {
        Bookmark           = KTextEditor::MarkInterface::markType01,
        Breakpoint         = KTextEditor::MarkInterface::markType02,
        ActiveBreakpoint   = KTextEditor::MarkInterface::markType03,
        ReachedBreakpoint  = KTextEditor::MarkInterface::markType04,
        DisabledBreakpoint = KTextEditor::MarkInterface::markType05,
        ExecutionPoint     = KTextEditor::MarkInterface::markType06
    };

    static Debugger *s_instance;
    KDevPartController *m_partController;
    QValueList<BPItem> BPList;
};

#endif
