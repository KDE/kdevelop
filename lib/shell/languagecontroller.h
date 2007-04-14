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
#ifndef LANGUAGECONTROLLER_H
#define LANGUAGECONTROLLER_H

#include "ilanguagecontroller.h"

namespace KParts {
class Part;
}

namespace KDevelop {

class KDEVPLATFORMSHELL_EXPORT LanguageController: public ILanguageController {
    Q_OBJECT
public:
    LanguageController(QObject *parent);
    ~LanguageController();

    void initialize();

    /** @copydoc ILanguageController::activeLanguages()*/
    virtual QList<ILanguage*> activeLanguages();
    /** @copydoc ILanguageController::language()*/
    virtual ILanguage *language(const QString &name);

private:
    Q_PRIVATE_SLOT(d, void activePartChanged(KParts::Part*))

    struct LanguageControllerPrivate *d;
};

}

#endif

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on;
