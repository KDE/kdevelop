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

#include "idealcentralwidget.h"

#include "idealcentrallayout.h"
#include "ideal.h"
#include "ideallayout.h"

using namespace Sublime;

IdealCentralWidget::IdealCentralWidget(IdealMainWidget * parent)
    : QWidget(parent)
    , m_mainWidget(parent)
{
    setLayout(new IdealCentralLayout(parent->mainWindow(), this));
}

Area * IdealCentralWidget::currentArea() const
{
    return centralLayout()->currentArea();
}

void IdealCentralWidget::setArea(Area * area)
{
    if (centralLayout()->currentArea())
        centralLayout()->currentArea()->disconnect(this);

    centralLayout()->setArea(area);

    connect(area, SIGNAL(viewAdded(Sublime::AreaIndex*, Sublime::View*)), this, SLOT(viewAdded(Sublime::AreaIndex*, Sublime::View*)));
    connect(area, SIGNAL(aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*)), this, SLOT(aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*)));
}

IdealCentralLayout* IdealCentralWidget::centralLayout() const
{
    return static_cast<IdealCentralLayout*>(layout());
}

void IdealCentralWidget::viewAdded(Sublime::AreaIndex * index, Sublime::View * view)
{
    Q_UNUSED(view);
    centralLayout()->refreshAreaIndex(index);
}

void IdealCentralWidget::aboutToRemoveView(Sublime::AreaIndex* index, Sublime::View* view)
{
    centralLayout()->removeView(index, view);
}

bool IdealCentralWidget::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        mainWidget()->centralWidgetFocused();

    return false;
}

IdealMainWidget * Sublime::IdealCentralWidget::mainWidget() const
{
    return m_mainWidget;
}

#include "idealcentralwidget.moc"
