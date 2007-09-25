%{H_TEMPLATE}
#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H


#include <kxmlguiwindow.h>

#include "ui_prefs_base.h"

class %{APPNAME}View;
class KPrinter;
class KToggleAction;
class KUrl;

/**
 * This class serves as the main window for %{APPNAME}.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Andreas Pakulat <apaku@gmx.de>
 * @version 0.1
 */
class %{APPNAME} : public KXmlGuiWindow
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
    void optionsPreferences();

private:
    void setupActions();

private:
    Ui::prefs_base ui_prefs_base ;
    %{APPNAME}View *m_view;

    KPrinter   *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _%{APPNAMELC}_H_
