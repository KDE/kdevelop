/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef ASTYLEPLUGIN_H
#define ASTYLEPLUGIN_H

#include <QMap>
#include <QVariant>
#include <QString>
#include <KUrl>

#include <interfaces/iplugin.h>

namespace KDevelop {
    class Context;
    class ContextMenuExtension;
}
class AStyleFormatter;

class QMenu;
class QWidget;
class QDomElement;
class KDialog;
class KAction;

namespace KParts {
    class Part;
}
namespace KTextEditor {
    class Cursor;
}

class AStylePlugin : public KDevelop::IPlugin
{
    Q_OBJECT

    public:
        AStylePlugin( QObject *parent, const QVariantList & = QVariantList() );
        ~AStylePlugin();

        /**
        * Format the selected files with the current style.
        */
//         void formatFiles(KUrl::List &list);

    private slots:
        void activePartChanged(KParts::Part *part);
        void beautifySource();
//         void formatFilesSelect();
       // void contextMenu(QMenu *popup, const KDevelop::Context *context);
        virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    
    private:
        void replaceSpacesWithTab(QString &input);

        KAction *m_formatTextAction;
        KAction *m_formatFileAction;

        // the configurable options.
        AStyleFormatter *m_formatter;
        KUrl::List m_urls;
};

#endif // ASTYLEPLUGIN_H
