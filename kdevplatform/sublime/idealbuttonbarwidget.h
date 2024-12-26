/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef IDEALBUTTONBARWIDGET_H
#define IDEALBUTTONBARWIDGET_H

#include <QWidget>

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

    bool isShown() const;
    void saveShowState();
    bool lastShowState();

    void loadOrderSettings(const KConfigGroup& configGroup);
    void saveOrderSettings(KConfigGroup& configGroup);

    bool isLocked() const;

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
    bool m_showState;
    QStringList m_buttonsOrder;
    IdealButtonBarLayout* m_buttonsLayout;
};

}

#endif // IDEALBUTTONBARWIDGET_H
