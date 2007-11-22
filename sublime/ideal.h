/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#ifndef SUBLIME_IDEAL_H
#define SUBLIME_IDEAL_H

#include <QtGui>

#include "ideallayout.h"

class KAction;
class KActionMenu;
class KActionCollection;

namespace Sublime {

class MainWindow;

class IdealToolButton: public QToolButton
{
    Q_OBJECT

    enum { DefaultButtonSize = 20 };

public:
    IdealToolButton(Qt::DockWidgetArea area, QWidget *parent = 0);

    Qt::Orientation orientation() const;

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    Qt::DockWidgetArea _area;
};

class IdealLabel : public QLabel
{
    Q_OBJECT

public:
    IdealLabel(Qt::Orientation orientation, const QString& text, QWidget* parent);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual int heightForWidth(int w) const;

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    Qt::Orientation m_orientation;
};

class IdealButtonBarWidget: public QWidget
{
    Q_OBJECT

public:
    IdealButtonBarWidget(Qt::DockWidgetArea area, class IdealMainWidget *parent = 0);

    KAction *addWidget(const QString& title, QDockWidget *widget);
    void showWidget(QDockWidget* widget);
    void removeAction(QAction* action);

    IdealMainWidget* parentWidget() const;

    Qt::Orientation orientation() const;

    QDockWidget* widgetForAction(QAction* action) const;

private Q_SLOTS:
    void showWidget(bool checked);
    void anchor(bool anchor);
    void maximize(bool maximized);

    void actionToggled(bool state);

protected:
    virtual void resizeEvent(QResizeEvent *event);

    virtual void actionEvent(QActionEvent *event);

private:
    Qt::DockWidgetArea _area;
    QHash<QAction *, IdealToolButton *> _buttons;
    QHash<QAction *, QDockWidget*> _widgets;
    QActionGroup* _actions;
};

class IdealDockWidgetTitle : public QWidget
{
    Q_OBJECT

public:
    IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget* parent, QAction* showAction);
    virtual ~IdealDockWidgetTitle();

    bool isAnchored() const;
    void setAnchored(bool anchored, bool emitSignals);

    bool isMaximized() const;
    void setMaximized(bool maximized);

Q_SIGNALS:
    void anchor(bool anchor);
    void maximize(bool maximize);
    void close();

private Q_SLOTS:
    void slotAnchor(bool anchored);
    void slotMaximize(bool maximized);

private:
    Qt::Orientation m_orientation;
    QToolButton* m_anchor;
    QToolButton* m_maximize;
};

class IdealCentralWidget : public QWidget
{
    Q_OBJECT

public:
    IdealCentralWidget(IdealMainWidget* parent);
    virtual ~IdealCentralWidget();

    IdealMainLayout* idealLayout() const;

protected:
    virtual void paintEvent(QPaintEvent* event);
};

class View;

class IdealMainWidget : public QWidget
{
    Q_OBJECT

public:
    IdealMainWidget(MainWindow* parent, KActionCollection* ac);

    // Public api
    void setCentralWidget(QWidget* widget);
    QAction* actionForView(View* view) const;
    void addView(Qt::DockWidgetArea area, View* View);
    void raiseView(View* view);
    void removeView(View* view);

    // Internal api

    // TODO can move the object filter here with judicious focusProxy?
    void centralWidgetFocused();

    void showDockWidget(QDockWidget* widget, bool show);
    void showDock(IdealMainLayout::Role role, bool show);

    void anchorDockWidget(QDockWidget* widget, bool anchor);

    IdealMainLayout* mainLayout() const;
    IdealCentralWidget* internalCentralWidget() const;

    void anchorDockWidget(bool checked, IdealButtonBarWidget* bar);
    void maximizeDockWidget(bool checked, IdealButtonBarWidget* bar);

    QWidget* firstWidget(IdealMainLayout::Role role) const;

    IdealButtonBarWidget* barForRole(IdealMainLayout::Role role) const;
    IdealMainLayout::Role roleForBar(IdealButtonBarWidget* bar) const;
    KAction* actionForRole(IdealMainLayout::Role role) const;

    void setAnchorActionStatus(bool checked);
    void setMaximizeActionStatus(bool checked);
    void setShowDockStatus(IdealMainLayout::Role role, bool checked);

public Q_SLOTS:
    void showLeftDock(bool show);
    void showRightDock(bool show);
    void showBottomDock(bool show);
    void showTopDock(bool show);
    void hideAllDocks();
    void anchorCurrentDock(bool anchor);
    void maximizeCurrentDock(bool maximized);
    void selectNextDock();
    void selectPreviousDock();

private:
    IdealButtonBarWidget *leftBarWidget;
    IdealButtonBarWidget *rightBarWidget;
    IdealButtonBarWidget *bottomBarWidget;
    IdealButtonBarWidget *topBarWidget;

    KAction* m_showLeftDock;
    KAction* m_showRightDock;
    KAction* m_showBottomDock;
    KAction* m_showTopDock;
    KAction* m_anchorCurrentDock;
    KAction* m_maximizeCurrentDock;
    KActionMenu* m_docks;

    IdealCentralWidget* mainWidget;
    class IdealMainLayout* m_mainLayout;

    QMap<QDockWidget*, Qt::DockWidgetArea> docks;
    QMap<View*, QAction*> views;
    QMap<QDockWidget*, QAction*> actions;
};

class IdealSplitterHandle : public QWidget
{
    Q_OBJECT

public:
    IdealSplitterHandle(Qt::Orientation orientation, QWidget* parent, IdealMainLayout::Role resizeRole);

Q_SIGNALS:
    void resize(int thickness, IdealMainLayout::Role resizeRole);
    
protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);

private:
    inline int convert(const QPoint& pos) const { return m_orientation == Qt::Horizontal ? pos.y() : pos.x(); }

    Qt::Orientation m_orientation;
    bool m_hover;
    int m_dragStart;
    IdealMainLayout::Role m_resizeRole;
};

}

#endif
