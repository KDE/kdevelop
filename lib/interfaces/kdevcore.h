/**
 * The interface to the application core
 */

#ifndef _KDEVCORE_H_
#define _KDEVCORE_H_

#include <qstringlist.h>

class KDialogBase;
class KDevPlugin;

class QStatusBar;
class QPopupMenu;

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
     * An enumeration that specifies the behaviour of the
     * methods gotoSourceFile() and gotoDocumentationFile().
     */
    enum Embedding { Replace, SplitHorizontal, SplitVertical };
    
    KDevCore( QObject *parent=0, const char *name=0 );
    ~KDevCore();

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
       close the current project and opens the new one
     */
    virtual void openProject(const QString& projectFileName)=0;
    /**
     * Goes to a given location in a source file and marks the line.
     * If fileName is null, the mark is cleared.
     * This is used by the debugger to mark the location where the
     * the debugger has stopped.
     */
    virtual void gotoExecutionPoint(const QString &fileName, int lineNum=0) = 0;
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
    virtual void running(KDevPlugin *which, bool runs) = 0;
   

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
