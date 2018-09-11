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

#ifndef KDEVPLATFORM_SUBLIME_IDEALLAYOUT_H
#define KDEVPLATFORM_SUBLIME_IDEALLAYOUT_H

#include <QLayout>

#include "sublimedefs.h"

namespace Sublime {

class IdealButtonBarLayout: public QLayout
{
    Q_OBJECT

public:
    explicit IdealButtonBarLayout(Qt::Orientation orientation, QWidget *parent = nullptr);

    ~IdealButtonBarLayout() override;

    void setHeight(int height);

    inline Qt::Orientation orientation() const;

    Qt::Orientations expandingDirections() const override;

    QSize minimumSize() const override;

    QSize sizeHint() const override;

    void setGeometry(const QRect &rect) override;

    void addItem(QLayoutItem *item) override;

    QLayoutItem* itemAt(int index) const override;

    QLayoutItem* takeAt(int index) override;

    int count() const override;

    void invalidate() override;

protected:
    int doVerticalLayout(const QRect &rect, bool updateGeometry = true) const;

    int doHorizontalLayout(const QRect &rect, bool updateGeometry = true) const;

    int buttonSpacing() const;

private:
    QList<QLayoutItem *> _items;
    const Qt::Orientation _orientation;
    int _height;
    mutable bool m_minSizeDirty : 1;
    mutable bool m_sizeHintDirty : 1;
    mutable bool m_layoutDirty : 1;
    mutable QSize m_min;
    mutable QSize m_hint;
};

}

#endif
