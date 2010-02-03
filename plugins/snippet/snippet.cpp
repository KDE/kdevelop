/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippet.h"

#include <KLocalizedString>
#include <KIcon>

Snippet::Snippet()
    : QStandardItem(i18n("<empty snippet>"))
{
    setIcon(KIcon("text-plain"));
}

Snippet::~Snippet()
{
}

QString Snippet::snippet() const
{
    return m_snippet;
}

void Snippet::setSnippet(const QString& snippet)
{
    m_snippet = snippet;
}

QString Snippet::prefix() const
{
    return m_prefix;
}

void Snippet::setPrefix(const QString& prefix)
{
    m_prefix = prefix;
}

QString Snippet::postfix() const
{
    return m_postfix;
}

void Snippet::setPostfix(const QString& postfix)
{
    m_postfix = postfix;
}

QString Snippet::arguments() const
{
    return m_arguments;
}

void Snippet::setArguments(const QString& arguments)
{
    m_arguments = arguments;
}
