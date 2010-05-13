/***************************************************************************
 *   Copyright (C) 2010 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTSOURCEPAGE_H
#define PROJECTSOURCEPAGE_H

#include <QWidget>

namespace Ui { class ProjectSourcePage; }
namespace KDevelop { class IPlugin; }
class KUrl;

class ProjectSourcePage : public QWidget
{
    Q_OBJECT
    public:
        ProjectSourcePage(const KUrl& initial, QWidget* parent = 0);
        
    private slots:
        void sourceChanged(int index);
        
    private:
        Ui::ProjectSourcePage* m_ui;
        QList<KDevelop::IPlugin*> m_plugins;
};

#endif // PROJECTSOURCEPAGE_H
