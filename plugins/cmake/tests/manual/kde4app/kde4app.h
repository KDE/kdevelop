/*
 * kde4app.h
 *
 * Copyright (C) 2008 %{AUTHOR} <%{EMAIL}>
 */
#ifndef KDE4APP_H
#define KDE4APP_H


#include <kxmlguiwindow.h>

#include "ui_prefs_base.h"

class kde4appView;
class QPrinter;
class KToggleAction;
class KUrl;

/**
 * This class serves as the main window for kde4app.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class kde4app : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    kde4app();

    /**
     * Default Destructor
     */
    virtual ~kde4app();

private slots:
    void fileNew();
    void optionsPreferences();

private:
    void setupActions();

private:
    Ui::prefs_base ui_prefs_base ;
    kde4appView *m_view;

    QPrinter   *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _KDE4APP_H_
