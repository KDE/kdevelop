/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
