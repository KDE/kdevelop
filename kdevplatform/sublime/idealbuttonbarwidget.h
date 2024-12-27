/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef IDEALBUTTONBARWIDGET_H
#define IDEALBUTTONBARWIDGET_H

#include <QWidget>

#include <memory>

class IdealToolButton;
class QAction;

class KConfigGroup;

namespace Sublime
{

class MainWindow;
class IdealController;
class IdealDockWidget;
class IdealButtonBarLayout;
class View;
class Area;

/**
 * This class keeps track of all checked tool view actions in an IdealButtonBarWidget. When none of the
 * actions is checked, this class stores the list of all actions unchecked during the last uncheck operation.
 */
class ILastCheckedActionsTracker
{
public:
    Q_DISABLE_COPY_MOVE(ILastCheckedActionsTracker)

    /**
     * @return whether at least one action is currently checked
     */
    [[nodiscard]] virtual bool isAnyChecked() const = 0;

    /**
     * Remember the current return value of isAnyChecked().
     */
    virtual void saveAnyCheckedState() = 0;
    /**
     * @return the return value of isAnyChecked() during the last call to saveAnyCheckedState()
     */
    [[nodiscard]] virtual bool lastSavedAnyCheckedState() const = 0;

    /**
     * Batch-uncheck all checked actions.
     */
    virtual void uncheckAll() = 0;

    /**
     * Batch-check all actions unchecked during the last uncheck operation.
     *
     * @return whether at least one action is checked now
     *
     * @note This function returns @c false only if there are no tracked actions to check.
     */
    virtual bool checkAllTracked() = 0;

    /**
     * If at least one action is currently checked but none of IdealDockWidget objects associated with checked actions
     * has the keyboard input focus, give the focus to the last shown IdealDockWidget associated with a checked action.
     *
     * @return whether the focus was given
     */
    virtual bool focusLastShownDockWidget() const = 0;

protected:
    ILastCheckedActionsTracker() = default;
    virtual ~ILastCheckedActionsTracker() = default;
};

class LastCheckedActionsTracker;

class IdealButtonBarWidget: public QWidget
{
    Q_OBJECT

public:
    IdealButtonBarWidget(Qt::DockWidgetArea area,
        IdealController *controller, Sublime::MainWindow *parent);

    /**
     * Add a tool view action for a given dock widget and tool view to this widget.
     *
     * Also update multiple properties of the dock widget.
     *
     * @param area the current sublime area
     * @param initiallyVisible whether the dock widget is currently visible, and
     *        consequently whether the added action should be initially checked
     *
     * @return the added action
     */
    QAction* addWidget(IdealDockWidget* widget, Area* area, View* view, bool initiallyVisible);

    void addAction(QAction *action);
    void removeAction(QAction* action);

    Qt::Orientation orientation() const;

    Qt::DockWidgetArea area() const;

    QWidget* corner() const;

    bool isEmpty() const;

    void loadOrderSettings(const KConfigGroup& configGroup);
    void saveOrderSettings(KConfigGroup& configGroup);

    /**
     * @return whether this dock is locked from hiding/restoring
     */
    bool isLocked() const;

    [[nodiscard]] ILastCheckedActionsTracker& lastCheckedActionsTracker() const;

Q_SIGNALS:
    void emptyChanged();

private:
    void showWidget(bool checked);

    void applyOrderToLayout();
    void takeOrderFromLayout();

    IdealToolButton* button(const QString& id) const;
    QString id(const IdealToolButton* button) const;

    void addButtonToOrder(const IdealToolButton* button);

    Qt::DockWidgetArea m_area;
    IdealController* m_controller;
    QWidget* m_corner;
    QStringList m_buttonsOrder;
    IdealButtonBarLayout* m_buttonsLayout;
    std::unique_ptr<LastCheckedActionsTracker> m_lastCheckedActionsTracker;
};

}

#endif // IDEALBUTTONBARWIDGET_H
