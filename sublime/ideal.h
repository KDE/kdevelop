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

class KAction;
class KActionCollection;

namespace Sublime {

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

class IdealButtonBarLayout: public QLayout
{
    Q_OBJECT

public:
    IdealButtonBarLayout(Qt::Orientation orientation, QWidget *parent = 0);

    virtual ~IdealButtonBarLayout();

    void setHeight(int height);

    inline Qt::Orientation orientation() const;

    virtual Qt::Orientations expandingDirections() const;

    virtual bool hasHeightForWidth() const;

    virtual int heightForWidth(int width) const;

    virtual QSize minimumSize() const;

    virtual QSize sizeHint() const;

    virtual void setGeometry(const QRect &rect);

    virtual void addItem(QLayoutItem *item);

    virtual QLayoutItem* itemAt(int index) const;

    virtual QLayoutItem* takeAt(int index);

    virtual int count() const;

    virtual void invalidate();

protected:
    int doVerticalLayout(const QRect &rect, bool updateGeometry = true) const;

    int doHorizontalLayout(const QRect &rect, bool updateGeometry = true) const;

private:
    QList<QLayoutItem *> _items;
    Qt::Orientation _orientation;
    int _height;
    mutable bool m_minSizeDirty : 1;
    mutable bool m_layoutDirty : 1;
    mutable QSize m_min;
};

class IdealButtonBarWidget: public QWidget
{
    Q_OBJECT

public:
    IdealButtonBarWidget(Qt::DockWidgetArea area, class IdealMainWidget *parent = 0);

    QAction *addWidget(const QString& title, QDockWidget *widget);
    void showWidget(QDockWidget* widget);

    IdealMainWidget* parentWidget() const;

    Qt::Orientation orientation() const;

private Q_SLOTS:
    void showWidget(bool checked);
    void anchor(bool anchor);

    void actionToggled(bool state);

protected:
    virtual void resizeEvent(QResizeEvent *event);

    virtual void actionEvent(QActionEvent *event);

private:
    Qt::DockWidgetArea _area;
    QHash<QWidgetAction *, IdealToolButton *> _buttons;
    QActionGroup* _actions;
};

class IdealDockWidgetTitle : public QWidget
{
    Q_OBJECT

public:
    IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget* parent, QAction* showAction);
    virtual ~IdealDockWidgetTitle();

    bool isAnchored() const;
    void setAnchored(bool anchored);

Q_SIGNALS:
    void anchor(bool anchor);
    void close();

private Q_SLOTS:
    void slotAnchor(bool anchor);
    
private:
    Qt::Orientation m_orientation;
    QToolButton* m_anchor;
};

class IdealMainLayout : public QLayout
{
    Q_OBJECT

public:
    enum Role {
        Left,
        Right,
        Bottom,
        Top,
        LeftSplitter,
        RightSplitter,
        BottomSplitter,
        TopSplitter,
        Central
    };

    IdealMainLayout(QWidget *parent = 0);

    virtual ~IdealMainLayout();

    void addWidget(QWidget* widget, Role role);
    QLayoutItem* itemForRole(Role role);
    QWidget* removeWidget(Role role, bool keepSplitter = false);
    void removeUnanchored();
    void showLastWidget(Role role, bool show);

    int splitterWidth() const;
    int widthForRole(Role role) const;

    bool isAreaAnchored(Role role) const;

    virtual QSize minimumSize() const;

    virtual QSize sizeHint() const;

    virtual void setGeometry(const QRect &rect);

    virtual void addItem(QLayoutItem *item);

    virtual QLayoutItem* itemAt(int index) const;

    virtual QLayoutItem* takeAt(int index);

    virtual int count() const;

    virtual void invalidate();

    QWidget* lastDockWidget() const;

public Q_SLOTS:
    void resizeWidget(int thickness, IdealMainLayout::Role resizeRole);
    void anchorWidget(bool anchor, IdealMainLayout::Role resizeRole);

protected:
    void doLayout(const QRect &rect, bool updateGeometry = true) const;

private:
    struct Settings
    {
        Settings();

        int width;
        bool anchored;
        QPointer<QWidget> last;
    };

    QMap<Role, QWidgetItem*> m_items;
    QHash<Role, Settings> m_settings;
    mutable bool m_layoutDirty, m_sizeHintDirty, m_minDirty;
    mutable QSize m_min, m_hint;
    QPointer<QWidget> m_lastDockWidget;
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

class IdealMainWidget : public QWidget
{
    Q_OBJECT

public:
    IdealMainWidget(QWidget* parent, KActionCollection* ac);

    void addWidget(Qt::DockWidgetArea area, const QString& title, QDockWidget* dock);
    void removeWidget(QDockWidget* dock);

    void setCentralWidget(QWidget* widget);

    // TODO can move the object filter here with judicious focusProxy?
    void centralWidgetFocused();

    void showDockWidget(QDockWidget* widget, bool show);
    void anchorDockWidget(QDockWidget* widget, bool anchor);

    IdealMainLayout* mainLayout() const;
    IdealCentralWidget* internalCentralWidget() const;

    void anchorDockWidget(bool checked, IdealButtonBarWidget* bar);

    QWidget* firstWidget(IdealMainLayout::Role role) const;

public Q_SLOTS:
    void showLeftDock(bool show);
    void showRightDock(bool show);
    void showBottomDock(bool show);
    void anchorCurrentDock(bool anchor);

private:
    IdealButtonBarWidget *leftBarWidget;
    IdealButtonBarWidget *rightBarWidget;
    IdealButtonBarWidget *bottomBarWidget;
    IdealButtonBarWidget *topBarWidget;

    KAction* m_raiseLeftDock;
    KAction* m_raiseRightDock;
    KAction* m_raiseBottomDock;

    IdealCentralWidget* mainWidget;
    class IdealMainLayout* m_mainLayout;

    QMap<QDockWidget*, Qt::DockWidgetArea> docks;
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
