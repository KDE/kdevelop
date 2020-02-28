/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakehelpdocumentation.h"
#include <QTreeView>
#include <KLocalizedString>
#include "cmakedoc.h"
#include "icmakedocumentation.h"
#include "cmakedocumentation.h"
#include "cmakecommandscontents.h"
#include <QEvent>
#include <QHeaderView>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

QString CMakeHomeDocumentation::name() const
{
    return i18n("CMake Content Page");
}

KDevelop::IDocumentationProvider* CMakeHomeDocumentation::provider() const
{
    return CMakeDoc::s_provider;
}

QWidget* CMakeHomeDocumentation::documentationWidget(KDevelop::DocumentationFindWidget*, QWidget* parent)
{
    auto* contents=new QTreeView(parent);
    contents->viewport()->installEventFilter(this);
    contents->header()->setVisible(false);

    contents->setModel(CMakeDoc::s_provider->model());
    QObject::connect(contents, &QTreeView::clicked, CMakeDoc::s_provider->model(), &CMakeCommandsContents::showItemAt);
    return contents;
}

bool CMakeHomeDocumentation::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
        event->ignore(); // Propagate to DocumentationView
    return QObject::eventFilter(watched, event);
}
