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

#include <QBoxLayout>

#include "sublimedefs.h"

namespace Sublime {

class IdealButtonBarLayout: public QBoxLayout
{
    Q_OBJECT

public:
    IdealButtonBarLayout(Qt::Orientation orientation, QWidget* styleParent);

    ~IdealButtonBarLayout() override;

    inline Qt::Orientation orientation() const;

    Qt::Orientations expandingDirections() const override;

protected:

    bool eventFilter(QObject* watched, QEvent* event) override;

    int buttonSpacing() const;

private:
    QWidget* const m_styleParentWidget;
    const Qt::Orientation m_orientation;
};

}

#endif
