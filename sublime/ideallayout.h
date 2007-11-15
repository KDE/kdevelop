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

#ifndef SUBLIME_IDEALLAYOUT_H
#define SUBLIME_IDEALLAYOUT_H

#include <QLayout>
#include <QPointer>
#include <QMap>
#include <QHash>

class QDockWidget;

class KAction;
class KActionCollection;

namespace Sublime {

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

    QDockWidget* lastDockWidget() const;
    QDockWidget* lastDockWidget(IdealMainLayout::Role role) const;

    class IdealMainWidget* mainWidget() const;

public Q_SLOTS:
    void resizeWidget(int thickness, IdealMainLayout::Role resizeRole);
    void anchorWidget(bool anchor, IdealMainLayout::Role resizeRole);

protected:
    void doLayout(QRect rect) const;
    void layoutItem(Role role, QRect& rect) const;

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
    int m_splitterWidth;
    QPointer<QWidget> m_lastDockWidget;
};

IdealMainLayout::Role roleForArea(Qt::DockWidgetArea area);

Qt::DockWidgetArea areaForRole(IdealMainLayout::Role role);

}

#endif
