/***************************************************************************
 *   Copyright 2004 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "profileeditor.h"

int main(int argc, char **argv)
{
    KAboutData about("kdevprofileeditor", 0, ki18n("KDevelop Profile Editor"), "1", KLocalizedString(),
                     KAboutData::License_GPL, ki18n("(c) 2004, The KDevelop Developers"));
    about.addAuthor(ki18n("Alexander Dymo"), KLocalizedString(), "adymo@kdevelop.org");
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    ProfileEditor editor;
    editor.show();

    return app.exec();
}
