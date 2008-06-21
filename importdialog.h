/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <KDialog>
#include <KUrl>
#include <KJob>

class ImportMetadataWidget;

class GitPlugin;

/**
 * Asks the user for all options needed to init an existing directory into
 * a Git repository. In IBasicVersionControl "import" term is used, that is why 
 * it is called import, but not init.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class ImportDialog : public KDialog
{
    Q_OBJECT
public:
    ImportDialog(GitPlugin *plugin, const KUrl& url, QWidget* parent=0);
    virtual ~ImportDialog();

public slots:
    virtual void accept();
    void jobFinished(KJob* job);

private:
    KUrl m_url;
    GitPlugin* m_plugin;
    ImportMetadataWidget* m_widget;
};

#endif
