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

#ifndef SUBLIME_IDEAL_CENTRAL_WIDGET_H
#define SUBLIME_IDEAL_CENTRAL_WIDGET_H

#include <QWidget>

#include "sublimeexport.h"

namespace Sublime {

class Area;
class AreaIndex;
class View;
class IdealMainWidget;
class IdealCentralLayout;

class SUBLIME_EXPORT IdealCentralWidget : public QWidget
{
    Q_OBJECT

public:
    IdealCentralWidget(IdealMainWidget* parent);

    IdealCentralLayout* centralLayout() const;
    IdealMainWidget* mainWidget() const;

    /**
     * Returns the area which was last setup over this widget.
     */
    Area* currentArea() const;

    /**
     * Set the current area to \a area
     */
    void setArea(Area* area);

public Q_SLOTS:
    /**
     * Notify that an area changed
     */
    void viewAdded(Sublime::AreaIndex* index, Sublime::View* view);
    void aboutToRemoveView(Sublime::AreaIndex* index, Sublime::View* view);

protected:
    virtual bool eventFilter(QObject *, QEvent *event);

private:
    IdealMainWidget* m_mainWidget;
};

}

#endif // SUBLIME_IDEAL_CENTRAL_WIDGET_H
