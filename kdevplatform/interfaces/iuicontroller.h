/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IUICONTROLLER_H
#define KDEVPLATFORM_IUICONTROLLER_H

#include "interfacesexport.h"

#include <QWidget>

class QAction;

namespace KParts {
    class MainWindow;
}
namespace Sublime{
    class Controller;
    class View;
    class Area;
    class Message;
}

namespace KDevelop {

class IDocument;
class IAssistant;

class KDEVPLATFORMINTERFACES_EXPORT IToolViewFactory {
public:
    IToolViewFactory() = default;
    virtual ~IToolViewFactory() {}
    Q_DISABLE_COPY_MOVE(IToolViewFactory)

    /**
     * called to create a new widget for this tool view
     * @param parent the parent to use as parent for the widget
     * @returns the new widget for the tool view
     */
    virtual QWidget* create(QWidget *parent = nullptr) = 0;
    /** 
     * @returns the identifier of this tool view.  The identifier
     * is used to remember which areas the tool view should appear
     * in, and must never change.
     */
    virtual QString id() const = 0;
    /**
     * @returns the default position where this tool view should appear
     */
    virtual Qt::DockWidgetArea defaultPosition() const = 0;
    /**
     * Fetch a list of actions to add to the toolbar of the tool view @p view
     * @param viewWidget the view to which the actions should be added
     * @returns a list of actions to be added to the toolbar
     */
    virtual QList<QAction*> toolBarActions( QWidget* viewWidget ) const { return viewWidget->actions(); }
    /**
     * Fetch a list of actions to be shown in the context menu of the tool view @p view.
     * The default implementation will return all actions of @p viewWidget.
     *
     * @param viewWidget the view for which the context menu should be shown
     * @returns a list of actions to be shown in the context menu
     */
    virtual QList<QAction*> contextMenuActions( QWidget* viewWidget ) const { return viewWidget->actions(); }

    /**
     * called when a new view is created from this template
     * @param view the new sublime view that is being shown
     */
    virtual void viewCreated(Sublime::View* view);

    /**
     * @returns if multiple tool views can by created by this factory in the same area.
     */
    virtual bool allowMultiple() const { return false; }
};

/**
 *
 * Allows to access various parts of the user-interface, like the tool views or the mainwindow
 */
class KDEVPLATFORMINTERFACES_EXPORT IUiController {
public:
    virtual ~IUiController();

    enum SwitchMode {
        ThisWindow /**< indicates that the area switch should be in this window */,
        NewWindow  /**< indicates that the area switch should be using a new window */
    };

    enum FindFlags {
        None = 0,
        Create = 1, ///The tool-view is created if it doesn't exist in the current area yet
        Raise = 2,  ///The tool-view is raised if it was found/created
        CreateAndRaise = Create | Raise ///The tool view is created and raised
    };

    virtual void switchToArea(const QString &areaName, SwitchMode switchMode) = 0;

    virtual void addToolView(const QString &name, IToolViewFactory *factory, FindFlags state = Create) = 0;
    virtual void removeToolView(IToolViewFactory *factory) = 0;
    
    /**  Makes sure that this tool-view exists in the current area, raises it, and returns the contained widget
       * Returns zero on failure */
    virtual QWidget* findToolView(const QString& name, IToolViewFactory *factory, FindFlags flags = CreateAndRaise) = 0;

    /**
     * Makes sure that the tool view that contains the widget @p toolViewWidget is visible to the user.
     */
    virtual void raiseToolView(QWidget* toolViewWidget) = 0;

    /** @return active mainwindow or 0 if no such mainwindow is active.*/
    virtual KParts::MainWindow *activeMainWindow() = 0;

    /*! @p status must implement KDevelop::IStatus */
    virtual void registerStatus(QObject* status) = 0;

    /**
     * This is meant to be used by IDocument subclasses to initialize the
     * Sublime::Document.
     */
    virtual Sublime::Controller* controller() = 0;

    /** Shows an error message in the status bar.
      *
      * Unlike all other functions in this class, this function is thread-safe.
      * You can call it from the background.
      *
      * @p message The message
      * @p timeout The timeout in seconds how long to show the message */
    virtual void showErrorMessage(const QString& message, int timeout = 5) = 0;
    // TODO: convert all these calls into postMessage

    /**
     * Shows a message in the message area.
     *
     * If running in NoGui mode, the message will be discarded.
     *
     * Unlike all other functions in this class, this function is thread-safe.
     * You can call it from the background.
     *
     * @p message the message, ownership is transferred
     */
    virtual void postMessage(Sublime::Message* message) = 0;

    /** @return area for currently active sublime mainwindow or 0 if
    no sublime mainwindow is active.*/
    virtual Sublime::Area *activeArea() = 0;

    /**
     * Widget which is currently responsible for consuming special events in the UI
     * (such as shortcuts)
     *
     * @sa IToolViewActionListener
     * @return QWidget implementing the IToolViewActionListener interface
     */
    virtual QWidget* activeToolViewActionListener() const = 0;

    /**
     * @returns all areas in the shell
     *
     * @note there will be one per mainwindow, of each type, plus the default ones.
     */
    virtual QList<Sublime::Area*> allAreas() const = 0;

protected:
    IUiController();
};

}

#endif

