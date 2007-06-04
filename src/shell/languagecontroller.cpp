/***************************************************************************
 *   Copyright (C) 2006 Adam Treat <treat@kde.org>                         *
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "languagecontroller.h"

#include <kparts/part.h>

#include "core.h"
#include "partcontroller.h"
#include "language.h"

namespace KDevelop {

struct LanguageControllerPrivate {
    LanguageControllerPrivate(LanguageController *controller) :m_controller(controller) {}

    void activePartChanged(KParts::Part *part)
    {
        KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(part);
        if (!ropart)
            return;

        KUrl url = ropart->url();
        if (!url.isValid())
            return;

        foreach (ILanguage *lang, activeLanguages)
            lang->deactivate();
        activeLanguages.clear();

        QList<Language*> languages = Language::findByUrl(url, m_controller);
        foreach (ILanguage *lang, languages)
        {
            lang->activate();
            activeLanguages << lang;
        }
    }

    QList<ILanguage*> activeLanguages;

private:
    LanguageController *m_controller;
};

LanguageController::LanguageController(QObject *parent)
    :ILanguageController(parent)
{
    d = new LanguageControllerPrivate(this);
}

LanguageController::~LanguageController()
{
    delete d;
}

void LanguageController::initialize()
{
    connect(Core::self()->partController(), SIGNAL(activePartChanged(KParts::Part*)),
        this, SLOT(activePartChanged(KParts::Part*)));
}

QList<ILanguage*> LanguageController::activeLanguages()
{
    return d->activeLanguages;
}

ILanguage *LanguageController::language(const QString &name)
{
    return Language::findByName(name);
}

}

#include "languagecontroller.moc"

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on;
