/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "isourceformattercontroller.h"

namespace KDevelop
{
ISourceFormatterController::ISourceFormatterController(QObject *parent ) 
        : QObject(parent)
{
}

ISourceFormatterController::~ISourceFormatterController()
{
}

}

#include "moc_isourceformattercontroller.cpp"
