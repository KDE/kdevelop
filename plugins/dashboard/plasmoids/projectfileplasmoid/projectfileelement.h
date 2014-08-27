/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/


#ifndef KALGEBRAPLASMA_HEADER
#define KALGEBRAPLASMA_HEADER

#include <Plasma/Applet>

class QSizeF;
class QLabel;

class ProjectFileItem : public Plasma::Applet
{
    Q_OBJECT
    public:
        ProjectFileItem(QObject *parent, const QVariantList &args);
        ~ProjectFileItem();
        
        void init();
       
        virtual void createConfigurationInterface(KConfigDialog* parent);
        virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;
    private:
        void reloadData();
        Plasma::TextBrowser *m_output;
        QVariantList m_args;
        Ui::Config m_ui;
        
    public slots:
        void configAccepted();
};

K_EXPORT_PLASMA_APPLET(projectfileelement, ProjectFileItem)

#endif
