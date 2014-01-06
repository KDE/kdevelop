/***************************************************************************
 *   Copyright 2011 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "flexibleaction.h"
#include <QApplication>
#include <QClipboard>
#include <QIcon>

using namespace KDevelop;

AbstractFunction::~AbstractFunction()
{}

FlexibleAction::FlexibleAction(const QIcon& icon, const QString& text, AbstractFunction* function, QObject* parent)
    : QAction(icon, text, parent)
    , m_function(function)
{
    connect(this, SIGNAL(triggered(bool)), SLOT(actionTriggered(bool)));
}

void FlexibleAction::actionTriggered(bool)
{
    (*m_function)();
}
