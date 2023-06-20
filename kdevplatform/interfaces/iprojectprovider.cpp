/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iprojectprovider.h"

namespace KDevelop
{

IProjectProviderWidget::IProjectProviderWidget(QWidget* parent)
    : QWidget(parent)
{}

IProjectProvider::~IProjectProvider()
{}

}

#include "moc_iprojectprovider.cpp"
