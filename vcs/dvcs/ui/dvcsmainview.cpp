/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "dvcsmainview.h"

#include <KLocale>
#include <KDebug>

#include "../dvcsplugin.h"
#include "dvcsgenericoutputview.h"

DVCSmainView::DVCSmainView(KDevelop::DistributedVersionControlPlugin *plugin, QWidget* parent )
 : QWidget( parent ),
   Ui::CvsMainViewBase(),
   m_plugin(plugin)
{
    Ui::CvsMainViewBase::setupUi(this);
    setWindowTitle(i18n("Git"));

    connect(m_plugin, SIGNAL(jobFinished(KJob*)),
            this, SLOT(slotJobFinished(KJob*)));

    connect(m_plugin, SIGNAL(addNewTabToMainView(QWidget*,QString)),
            this, SLOT(slotAddTab(QWidget*,QString)) );

    // create a default output view
    m_mainview = new DVCSgenericOutputView(m_plugin);
    tabwidget->addTab( m_mainview, i18n("DVCS") );

    // add a close button as corner widget
    m_closeButton = new QToolButton(tabwidget);
    m_closeButton->setIcon( KIcon( "tab-close" ) );
    m_closeButton->adjustSize();
    m_closeButton->setAutoRaise(true);
    m_closeButton->setEnabled(false);
    tabwidget->setCornerWidget( m_closeButton );
    connect(m_closeButton, SIGNAL(clicked()),
            this, SLOT(slotTabClose()));
}

DVCSmainView::~DVCSmainView()
{
    delete m_mainview;
}

void DVCSmainView::slotAddTab(QWidget * tab, const QString& label)
{
    kDebug() << "adding tab:" << label;

    int idx = tabwidget->addTab( tab, label );
    tabwidget->setCurrentIndex(idx);

    if (tabwidget->count() > 1)
        m_closeButton->setEnabled(true);
}

void DVCSmainView::slotJobFinished(KJob * job)
{
    m_mainview->slotJobFinished(job);
    tabwidget->setCurrentIndex(0);
}

void DVCSmainView::slotTabClose()
{
    int idx = tabwidget->currentIndex();

    // don't allow to close the first tab
    if (idx != 0)
        tabwidget->removeTab( idx );

    // if only the first tab remains, disable the close button
    if (tabwidget->count() <= 1)
        m_closeButton->setEnabled(false);
}
