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
class KToggleAction;
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

private slots:
    void fileNew();
    void optionsShowToolbar();
    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void optionsPreferences();
    void newToolbarConfig();

    void changeStatusbar(const QString& text);
    void changeCaption(const QString& text);

private:
    void setupAccel();
    void setupActions();

private:
    %{APPNAME}View *m_view;

    KPrinter   *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _%{APPNAMEUC}_H_

