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

namespace Sublime {

enum IdealButtonBarArea { // ### move me
    LeftButtonBarArea,
    RightButtonBarArea,
    TopButtonBarArea,
    BottomButtonBarArea
};

class IdealToolButton: public QToolButton
{
    Q_OBJECT

    enum { DefaultButtonSize = 20 };

public:
    IdealToolButton(IdealButtonBarArea area, QWidget *parent = 0);

    Qt::Orientation orientation() const;

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    IdealButtonBarArea _area;
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
    IdealButtonBarWidget(IdealButtonBarArea area, class IdealMainWidget *parent = 0);

    QAction *addWidget(QDockWidget *widget);
    void showWidget(QDockWidget* widget);

    Qt::Orientation orientation() const;

public Q_SLOTS:
    void closeAll();

private Q_SLOTS:
    void _k_showWidget(bool checked);

protected:
    void reposition();

    void reposition(QWidget *widget);

    virtual void resizeEvent(QResizeEvent *event);

    virtual void actionEvent(QActionEvent *event);

private:
    IdealButtonBarArea _area;
    QHash<QWidgetAction *, IdealToolButton *> _buttons;
    QSplitter* resizeHandle;
};

class IdealDockWidgetTitle : public QWidget
{
    Q_OBJECT

public:
    IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget* parent, IdealMainWidget* main);
    virtual ~IdealDockWidgetTitle();

private:
    Qt::Orientation m_orientation;
};

class IdealCentralWidget : public QWidget
{
    Q_OBJECT

public:
    IdealCentralWidget(IdealMainWidget* parent);
    virtual ~IdealCentralWidget();

protected:
    virtual void paintEvent(QPaintEvent* event);
};

class IdealMainWidget : public QWidget
{
    Q_OBJECT

public:
    IdealMainWidget(QWidget* parent);

    void addWidget(Qt::DockWidgetArea area, QDockWidget* dock);
    void removeWidget(QDockWidget* dock);

    void setCentralWidget(QWidget* widget);

    // TODO can move the object filter here with judicious focusProxy?
    void centralWidgetFocused();

public Q_SLOTS:
    void anchorDockWidget(bool checked);
    
private:
    IdealButtonBarWidget *leftBarWidget;
    IdealButtonBarWidget *rightBarWidget;
    IdealButtonBarWidget *bottomBarWidget;
    IdealButtonBarWidget *topBarWidget;

    IdealCentralWidget* mainWidget;
    class IdealMainLayout* mainLayout;

    QMap<QDockWidget*, Qt::DockWidgetArea> docks;
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
        Central
    };

    IdealMainLayout(QWidget *parent = 0);

    virtual ~IdealMainLayout();

    void addWidget(Role role, QWidget* widget);
    QWidget* removeWidget(Role role);

    virtual QSize minimumSize() const;

    virtual QSize sizeHint() const;

    virtual void setGeometry(const QRect &rect);

    virtual void addItem(QLayoutItem *item);

    virtual QLayoutItem* itemAt(int index) const;

    virtual QLayoutItem* takeAt(int index);

    virtual int count() const;

    virtual void invalidate();

protected:
    void doLayout(const QRect &rect, bool updateGeometry = true) const;

private:
    QMap<Role, QLayoutItem *> m_items;
    mutable bool m_layoutDirty;
    mutable QSize m_min, m_hint;
};

/*class IdealDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    IdealDockWidget(const QString& title, QWidget* parent);

protected:
    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);
};*/

}

#endif
