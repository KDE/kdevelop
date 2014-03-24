/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_UICONTROLLER_H
#define KDEVPLATFORM_UICONTROLLER_H

#include "shellexport.h"

#include <interfaces/iuicontroller.h>
#include <sublime/controller.h>

#include <ksharedconfig.h>
#include <interfaces/iassistant.h>

class QListWidgetItem;

namespace Sublime {
    class AreaIndex;
    class ToolDocument;
}

namespace KDevelop {

class Core;
class MainWindow;

class KDEVPLATFORMSHELL_EXPORT UiController: public Sublime::Controller, public IUiController
{
    Q_OBJECT

public:
    UiController(Core *core);
    virtual ~UiController();

    /** @return area for currently active sublime mainwindow or 0 if
    no sublime mainwindow is active.*/
    virtual Sublime::Area *activeArea();
    /** @return active sublime mainwindow or 0 if no such mainwindow is active.*/
    virtual Sublime::MainWindow *activeSublimeWindow();
    /** @return active sublime mainwindow or 0 if no such mainwindow is active.*/
    virtual KParts::MainWindow *activeMainWindow();

    /** @return default main window - the main window for default area in the shell.
    No guarantee is given that it always exists so this method may return 0.*/
    MainWindow *defaultMainWindow();
    /** @return the default area for this shell.*/
    Sublime::Area *defaultArea();

    virtual void switchToArea(const QString &areaName, SwitchMode switchMode);

    virtual void addToolView(const QString &name, IToolViewFactory *factory);
    virtual void removeToolView(IToolViewFactory *factory);

    virtual QWidget* findToolView(const QString& name, IToolViewFactory *factory, FindFlags flags);
    virtual void raiseToolView(QWidget* toolViewWidget);

    void selectNewToolViewToAdd(MainWindow *mw);

    void initialize();
    void cleanup();

    void showSettingsDialog();
    Sublime::Controller* controller();

    void mainWindowDeleted(MainWindow* mw);

    void saveAllAreas(KSharedConfig::Ptr config);
    void saveArea(Sublime::Area* area, KConfigGroup & group);
    void loadAllAreas(KSharedConfig::Ptr config);
    void loadArea(Sublime::Area* area, const KConfigGroup & group);

    /*! @p status must implement KDevelop::IStatus */
    virtual void registerStatus(QObject* status);

    virtual void popUpAssistant(const KDevelop::IAssistant::Ptr& assistant);

    virtual void showErrorMessage(const QString& message, int timeout);

    /// Returns list of available view factories together with their ToolDocuments.
    /// @see addToolView(), removeToolView(), findToolView()
    const QMap<IToolViewFactory*, Sublime::ToolDocument*>& factoryDocuments() const;

    /// Adds a tool view in the active area to the dock area @p area.
    /// @see activeArea()
    void addToolViewToDockArea(KDevelop::IToolViewFactory* factory, Qt::DockWidgetArea area);

    bool toolViewPresent(Sublime::ToolDocument* doc, Sublime::Area* area);

public Q_SLOTS:
    void raiseToolView(Sublime::View * view);

private Q_SLOTS:
    void assistantHide();
    void assistantActionsChanged();
    void addNewToolView(MainWindow* mw, QListWidgetItem* item);
    void hideAssistant();

private:
    void addToolViewIfWanted(IToolViewFactory* factory,
                           Sublime::ToolDocument* doc,
                           Sublime::Area* area);
    Sublime::View* addToolViewToArea(IToolViewFactory* factory,
                           Sublime::ToolDocument* doc,
                           Sublime::Area* area, Sublime::Position p=Sublime::AllPositions);
    void setupActions();
    class UiControllerPrivate* const d;
    friend class UiControllerPrivate;
    Q_PRIVATE_SLOT(d, void widgetChanged(QWidget*,QWidget*))
};

}

#endif

