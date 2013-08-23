/***************************************************************************
 *   Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
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

#ifndef AREADISPLAY_H
#define AREADISPLAY_H

#include <QWidget>


class QPushButton;
class QMenu;

namespace KDevelop {
    class MainWindow;
}
namespace Sublime {
    class MainWindow;
    class Area;
}

/**
 * This widget displays the current area by the menu 
 */

class AreaDisplay : public QWidget
{
    Q_OBJECT
    public:
        explicit AreaDisplay(KDevelop::MainWindow* parent);

        virtual QSize sizeHint() const;
        virtual QSize minimumSizeHint() const;

    private slots:
        void newArea(Sublime::Area* area);
        void backToCode();

    private:
        QMenu* m_menu;
        QPushButton* m_button;
        KDevelop::MainWindow* m_mainWindow;
};

#endif // AREADISPLAY_H
