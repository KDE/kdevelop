/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>
  Copyright 2011 Alexander Dymo <adymo@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IDEALBUTTONBARWIDGET_H
#define IDEALBUTTONBARWIDGET_H

#include <QWidget>
#include <QHash>

class IdealToolButton;
class ToolViewAction;
class QAction;

class KConfigGroup;

namespace Sublime
{

class MainWindow;
class IdealController;
class IdealDockWidget;
class View;
class Area;

class IdealButtonBarWidget: public QWidget
{
    Q_OBJECT

public:
    IdealButtonBarWidget(Qt::DockWidgetArea area,
        IdealController *controller, Sublime::MainWindow *parent);

    QAction* addWidget(IdealDockWidget *widget,
                       Area* area, View *view);

    void addAction(QAction *action);
    void removeAction(QAction* action);

    Qt::Orientation orientation() const;

    Qt::DockWidgetArea area() const;

    IdealDockWidget* widgetForAction(QAction* action) const;

    QWidget* corner();

    void showWidget(QAction *widgetAction, bool checked);
    bool isEmpty();

    bool isShown();
    void saveShowState();
    bool lastShowState();

    void loadOrderSettings(const KConfigGroup& configGroup);
    void saveOrderSettings(KConfigGroup& configGroup);

    bool isLocked();

signals:
    void emptyChanged();

private:
    void showWidget(bool checked);

    void applyOrderToLayout();
    void takeOrderFromLayout();

    IdealToolButton* button(const QString& id) const;
    QString id(const IdealToolButton* button) const;

    void addButtonToOrder(const IdealToolButton* button);

    Qt::DockWidgetArea _area;
    IdealController *_controller;
    QWidget *_corner;
    bool _showState;
    QStringList _buttonsOrder;
};

}

#endif // IDEALBUTTONBARWIDGET_H
