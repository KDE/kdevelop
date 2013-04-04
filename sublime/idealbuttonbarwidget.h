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
class KAction;
namespace Sublime {

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

    KAction *addWidget(const QString& title, IdealDockWidget *widget,
                       Area* area, View *view);
    void removeAction(QAction* action);

    Qt::Orientation orientation() const;

    Qt::DockWidgetArea area() const;

    IdealDockWidget* widgetForAction(QAction* action) const;

    QWidget* corner();

    void showWidget(QAction *widgetAction, bool checked);
    bool isEmpty();

private Q_SLOTS:
    void showWidget(bool checked);
    void actionToggled(bool state);

signals:
    void emptyChanged();

protected:
    virtual void actionEvent(QActionEvent *event);

private:
    Sublime::MainWindow* parentWidget() const;

    Qt::DockWidgetArea _area;
    IdealController *_controller;
    QHash<QAction *, IdealToolButton*> _buttons;
    QHash<QAction *, IdealDockWidget*> _widgets;
    QWidget *_corner;
};

}

#endif // IDEALBUTTONBARWIDGET_H
