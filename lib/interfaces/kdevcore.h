/**
 * The interface to the application core
 */

#ifndef _KDEVCORE_H_
#define _KDEVCORE_H_

#include <qstringlist.h>
#include "kdevpart.h"

class KDialogBase;

namespace KEditor {
  class Editor;
}


class QStatusBar;


class Context
{
public:
    Context(const QCString &type)
    { m_type = type; }
    ~Context() {}
    
    bool hasType(const QCString &type) const
    { return type == m_type; }
    
private:
    QCString m_type;
};


/**
 * A context for the popup menu in the editor. In this case,
 * linestr() returns the contents of the line in which the
 * mouse button has been pressed, and col() returns the column
 * number.
 */
class EditorContext : public Context
{
public:
    EditorContext(const QString &linestr, int col)
        : Context("editor"), m_linestr(linestr), m_col(col) {}
    ~EditorContext() {}

    QString linestr() const
    { return m_linestr; }
    int col() const
    { return m_col; }

private:
    QString m_linestr;
    int m_col;
};


/**
 * A context for the popup menu in the html widget. In this
 * case, url() returns the URL of the page currently shown,
 * and selection() returns the selected text.
 */
class DocumentationContext : public Context
{
public:
    DocumentationContext(const QString &url, const QString &selection)
        : Context("documentation"), m_url(url), m_selection(selection) {}
    ~DocumentationContext() {}

    QString url() const
    { return m_url; }
    QString selection() const
    { return m_selection; }
    
private:
    QString m_url;
    QString m_selection;
};


/**
 * A context for the popup menu in file views. Here,
 * fileName() returns the absolute name of the selected
 * file or directory.
 */
class FileContext : public Context
{
public:
    FileContext(const QString &fileName)
        : Context("file"), m_fileName(fileName) {}
    ~FileContext() {}

    QString fileName() const
    { return m_fileName; }

private:
    QString m_fileName;
};

/**
 * A context for the popup menu in class views. In this case,
 * className() returns the name of the class, including its
 * scope (i.e. namespace).
 */
class ClassContext : public Context
{
public:
    ClassContext(const QString &classname)
        : Context("class"), m_classname(classname) {}
    ~ClassContext() {}

    QString classname() const
    { return m_classname; }

private:
    QString m_classname;
};


class KDevCore : public QObject
{
    Q_OBJECT
    
public:
    /**
     * An enumeration used for the widget embedding. A SelectView
     * appears on the left side of the main window. An OutputView
     * appears at the bottom. DocumentView is not currently
     * supported.
     */
    enum Role { SelectView, OutputView, DocumentView };
    /**
     * An enumeration that specifies the behaviour of the
     * methods gotoSourceFile() and gotoDocumentationFile().
     */
    enum Embedding { Replace, SplitHorizontal, SplitVertical };
    
    KDevCore( QObject *parent=0, const char *name=0 );
    ~KDevCore();

    /**
     * Embed the given widget into the user interface.
     */
    virtual void embedWidget(QWidget *, Role, const QString&) = 0;
    /**
     * Make the given widget visible. Useful e.g. for the compiler
     * frontend which wants to be shown in the foreground when
     * a command is started.
     */
    virtual void raiseWidget(QWidget *) = 0;
    /**
     * This method should be called by a part that wants to show a
     * context menu. The parameter context should be filled with
     * information about the context in which this happens (see
     * EditorContext, DocumentationContext, ClassContext, ...).
     * Essentially, this method emits the signal contextMenu()
     * which other parts can use to hook in.
     */
    virtual void fillContextMenu(QPopupMenu *popup, const Context *context) = 0;
    /**
     * "Goes" to a file. This is a generic method that is used
     * e.g. by file trees. For non-text files (i.e. files which
     * have a mime-type that doesn't start with "text/") it runs
     * the respective application. Text files are loaded into an
     * editor via # gotoSourceFile().
     */
    virtual void gotoFile(const KURL &url) = 0;
    /**
     * Loads a file into the HTML viewer.
     */
    virtual void gotoDocumentationFile(const KURL& url,
                                       Embedding embed=Replace) = 0;
    /**
     * Loads a file into the editor and jump to a line number.
     */
    virtual void gotoSourceFile(const KURL& url, int lineNum=0,
                                Embedding embed=Replace) = 0;
    /**
     * Goes to a given location in a source file and marks the line.
     * If fileName is null, the mark is cleared.
     * This is used by the debugger to mark the location where the
     * the debugger has stopped.
     */
    virtual void gotoExecutionPoint(const QString &fileName, int lineNum=0) = 0;
    /**
     * Saves all modified buffers.
     */
    virtual void saveAllFiles() = 0;
    /**
     * Reverts all modified buffers to their version on disk.
     */
    virtual void revertAllFiles() = 0;
    /**
     * Sets a breakpoint in the editor document belong to fileName.
     * If id==-1, the breakpoint is deleted.
     */
    virtual void setBreakpoint(const QString &fileName, int lineNum,
                               int id, bool enabled, bool pending) = 0;
    /**
     * Marks the component as running (or not running). As long as at least one
     * component is running, the stop button is enabled. When it is pressed,
     * all components get a stopButtonClicked().
     */
    virtual void running(KDevPart *which, bool runs) = 0;
    /**
     * Puts a message in the status bar.
     */
    virtual void message(const QString &str) = 0;
   

	/**
	 * The manager used to manage the parts.
	 */
    virtual KParts::PartManager *partManager() const = 0;	


    /**
     * The interface to the editor used
     */
    virtual KEditor::Editor *editor() = 0;


    /**
     * The statusbar used
     */
    virtual QStatusBar *statusBar() const = 0;

      
signals:

    /**
     * Emitted after the core has done all initializations and
     * the main window has been shown.
     */
    void coreInitialized();
    /**
     * A project has been opened
     */
    void projectOpened();
    /**
     * The project is about to be closed.
     */
    void projectClosed();
    /**
     * Emitted when a file has been saved.
     */
    void savedFile(const QString &fileName);
    /**
     * Emitted when a file has been loaded.
     */
    void loadedFile(const QString &fileName);
    /**
     * The user has toggled a breakpoint.
     */
    void toggledBreakpoint(const QString &fileName, int lineNum);
    /*
     * The user wants to edit the properties of a breakpoint.
     */
    void editedBreakpoint(const QString &fileName, int lineNum);
    /**
     * The user wants to enable/disable a breakpoint.
     */
    void toggledBreakpointEnabled(const QString &fileName, int lineNum);
    
    /**
     * The user has clicked the stop button.
     */
    void stopButtonClicked();
    /**
     * A context menu has been requested somewhere. Components
     * may hook some entries into it. More information on the
     * context can be obtained by looking for the type of
     * the context parameter and casting it accordingly.
     * 
     */
    void contextMenu(QPopupMenu *popup, const Context *context);
    /**
     * Expects that a configuration page for use in the
     * KDevelop settings dialog is created by the component.
     */
    void configWidget(KDialogBase *dlg);
    /**
     * Expects that a configuration page for use in the
     * Project settings dialog is created by the component.
     */
    void projectConfigWidget(KDialogBase *dlg);
};

#endif
