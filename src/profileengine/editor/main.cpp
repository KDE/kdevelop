/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "profileeditor.h"

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kdevprofileeditor", I18N_NOOP("KDevelop Profile Editor"), "1", "",
                     KAboutData::License_GPL, "(c) 2004, The KDevelop Developers", 0, 0, "");
    about.addAuthor("Alexander Dymo", 0, "adymo@kdevelop.org");
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app(argc, argv);
    ProfileEditor editor;
    editor.show();
    app.setMainWidget(&editor);
    
    return app.exec();
}