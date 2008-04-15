/*
  Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#ifndef SUBLIME_IDEAL_CENTRAL_LAYOUT_H
#define SUBLIME_IDEAL_CENTRAL_LAYOUT_H

#include <QLayout>
#include <QWidgetItem>
#include <QHash>

#include "area.h"
#include "sublimeexport.h"

class QSplitterHandle;

namespace Sublime {

class IdealCentralWidget;
class IdealSplitterHandle;
class Switcher;
class MainWindow;

class SUBLIME_EXPORT AreaLayout
{
public:
    AreaLayout(AreaLayout* parent = 0);
    virtual ~AreaLayout();

    void clearChildren();
    void clearViews();

    template <typename Operator>
    Area::WalkerMode walkLayout(Operator &op)
    {
        Area::WalkerMode mode = op(this);
        if (mode == Area::StopWalker)
            return mode;

        if (firstChild && secondChild)
        {
            mode = firstChild->walkLayout(op);
            if (mode == Area::StopWalker)
                return mode;

            mode = secondChild->walkLayout(op);
        }

        return mode;
    }

    AreaIndex* source;

    // Info for split areas
    AreaLayout* parent;
    AreaLayout* firstChild;
    AreaLayout* secondChild;
    QWidgetItem* splitter;
    Qt::Orientation orientation;
    double splitterPercentage;
    QRect currentRect;

    // Info for end-areas
    QList<View*> views;
    QHash<View*, QWidgetItem*> viewItems;

    Switcher* switcher;
    QWidgetItem* switcherItem;
    View* activeView;
};

class SUBLIME_EXPORT IdealCentralLayout : public QLayout
{
    Q_OBJECT

public:
    IdealCentralLayout(MainWindow* mw, IdealCentralWidget* parent);

    void clear();

    Area* currentArea() const;
    void setArea(Area* area);
    void setupArea(AreaIndex* area, AreaLayout* layout);
    void removeView(AreaIndex* area, View* view);

    // Only for tests
    AreaLayout* topLayout() const;

    // Reimplementations for QLayout
    virtual void addItem ( QLayoutItem * item );
    virtual int count () const;
    virtual int indexOf ( QWidget * widget ) const;
    virtual QLayoutItem * itemAt ( int index ) const;
    virtual QSize minimumSize () const;
    virtual QLayoutItem * takeAt ( int index );

    // Reimplementations for QLayoutItem
    virtual void setGeometry ( const QRect & r );
    virtual QSize sizeHint () const;

public Q_SLOTS:
    void refreshAreaIndex(Sublime::AreaIndex* area);

private Q_SLOTS:
    void widgetActivated(int index);
    void activateView(Sublime::View* view);
    void resize(int thickness, int width, QVariant data);
    void aboutToRemoveAreaIndex(Sublime::AreaIndex* index);

private:
    void createViewItem(AreaLayout* layout, View* view);

    void doLayout(AreaLayout* layout, const QRect& rect);
    QSize doMin(AreaLayout* layout) const;
    QSize doHint(AreaLayout* layout) const;

    void activateView(AreaLayout* layout, View* view);

    mutable bool m_layoutDirty, m_sizeHintDirty, m_minDirty;
    mutable QSize m_min, m_hint;
    int m_splitterWidth;

    Area* m_currentArea;
    AreaLayout* m_topArea;
};

}

#endif // SUBLIME_IDEAL_CENTRAL_LAYOUT_H
