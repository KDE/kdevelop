/* This file is part of KDevelop
   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
   
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

#ifndef CPPCHECKGENERICCONFIGPAGE_H_
#define CPPCHECKGENERICCONFIGPAGE_H_

#include <QObject>
#include <kcmodule.h>
#include <ksharedconfig.h>
class KIcon;

namespace cppcheck
{
class Plugin;

namespace Ui
{
class GenericConfig;
}

class   GenericConfigPage : public KCModule
{
    Q_OBJECT

public:

    GenericConfigPage(QWidget *parent, const QVariantList &args = QVariantList() );
    ~GenericConfigPage();
    
    virtual KIcon icon() const;
    virtual QString title() const;

signals:

public slots:
    void save();
    void load();

private:
    Ui::GenericConfig *ui;
};

}

#endif /* CPPCHECKGENERICCONFIGPAGE_H_ */
