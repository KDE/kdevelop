/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef IDEALDOCKWIDGET_H
#define IDEALDOCKWIDGET_H

#include <QDockWidget>

class QMainWindow;

namespace Sublime {
class Area;
class IdealController;
class View;

class IdealDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit IdealDockWidget(IdealController* controller, QMainWindow* parent);
    ~IdealDockWidget() override;

    Area *area() const;
    void setArea(Area *area);

    View *view() const;
    void setView(View *view);

    Qt::DockWidgetArea dockWidgetArea() const;
    void setDockWidgetArea(Qt::DockWidgetArea dockingArea);

public Q_SLOTS:
    /// The IdealToolButton also connects to this slot to show the same context menu.
    void contextMenuRequested(const QPoint &point);

Q_SIGNALS:
    void closeRequested();

private Q_SLOTS:
    void slotRemove();

private:
    Area *m_area;
    View *m_view;
    Qt::DockWidgetArea m_docking_area;
    IdealController* const m_controller;
};

}

#endif // IDEALDOCKWIDGET_H
