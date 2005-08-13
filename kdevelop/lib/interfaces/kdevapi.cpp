/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kdevapi.h"

#include "kdevcoderepository.h"

///////////////////////////////////////////////////////////////////////////////
// class KDevApi::Private
///////////////////////////////////////////////////////////////////////////////

class KDevApi::Private
{
public:
    Private()
        : m_projectDom(0), m_project(0), m_languageSupport(0),
        m_codeRepository(0)
    {}
    
    QDomDocument *m_projectDom;
    KDevProject  *m_project;
    KDevLanguageSupport *m_languageSupport;
    KDevCodeRepository* m_codeRepository;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevApi
///////////////////////////////////////////////////////////////////////////////

KDevApi::KDevApi()
{
    d = new KDevApi::Private;
    d->m_codeRepository = new KDevCodeRepository();
}

KDevApi::~KDevApi()
{
    delete d->m_codeRepository;
    delete d;
}

KDevProject *KDevApi::project() const
{
  return d->m_project;
}

void KDevApi::setProject(KDevProject *project)
{
  d->m_project = project;
}

KDevLanguageSupport *KDevApi::languageSupport() const
{
  return d->m_languageSupport;
}

void KDevApi::setLanguageSupport(KDevLanguageSupport *languageSupport)
{
  d->m_languageSupport = languageSupport;
}

QDomDocument *KDevApi::projectDom() const
{
    return d->m_projectDom;
}

void KDevApi::setProjectDom(QDomDocument *dom)
{
    d->m_projectDom = dom;
}

KDevCodeRepository *KDevApi::codeRepository() const
{
    return d->m_codeRepository;
}

#include "kdevapi.moc"
