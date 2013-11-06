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

#ifndef IDEALDOCKWIDGET_H
#define IDEALDOCKWIDGET_H

#include <QDockWidget>
#include "idealcontroller.h"

namespace Sublime {
class IdealDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    IdealDockWidget(IdealController *controller, Sublime::MainWindow *parent);
    virtual ~IdealDockWidget();

    Area *area() const;
    void setArea(Area *area);

    View *view() const;
    void setView(View *view);

    Qt::DockWidgetArea dockWidgetArea() const;
    void setDockWidgetArea(Qt::DockWidgetArea dockingArea);

public slots:
    /// The IdealToolButton also connects to this slot to show the same context menu.
    void contextMenuRequested(const QPoint &point);

Q_SIGNALS:
    void closeRequested();

private Q_SLOTS:
    void slotRemove();

private:
    Qt::Orientation m_orientation;
    Area *m_area;
    View *m_view;
    Qt::DockWidgetArea m_docking_area;
    IdealController *m_controller;
};

}

#endif // IDEALDOCKWIDGET_H
