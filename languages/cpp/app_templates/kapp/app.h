//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>
%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>

#include "%{APPNAMELC}view.h"

class KPrinter;
class KURL;

/**
 * This class serves as the main window for %{APPNAME}.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class %{APPNAME} : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    %{APPNAME}();

    /**
     * Default Destructor
     */
    virtual ~%{APPNAME}();

    /**
     * Use this method to load whatever file/URL you have
     */
    void load(const KURL& url);

protected:
    /**
     * Overridden virtuals for Qt drag 'n drop (XDND)
     */
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

protected:
    /**
     * This function is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig *);

    /**
     * This function is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig *);


private slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void filePrint();
    void optionsPreferences();

    void changeStatusbar(const QString& text);
    void changeCaption(const QString& text);

private:
    void setupAccel();
    void setupActions();

private:
    %{APPNAME}View *m_view;

    KPrinter   *m_printer;
};

#endif // _%{APPNAMEUC}_H_

