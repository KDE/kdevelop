/* KDevelop Run Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
* Copyright 2008  Aleix Pol <aleixpol@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#ifndef ASKTARGETNAME_H
#define ASKTARGETNAME_H

#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>

class AskTargetName : public QDialog
{
    public:
        AskTargetName(QWidget* parent) : QDialog(parent)
        {
            QDialogButtonBox * buttonBox;
            QVBoxLayout *items=new QVBoxLayout(this);
            items->addWidget(new QLabel(i18n("Enter a name for the target"), this));
            items->addWidget(edit=new QLineEdit(this));
    //         items->addItem(new QSpacerItem());
            items->addWidget(buttonBox=new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, this));
            
            connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
            connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        }
        
        QString name() const { return edit->text(); }
    private:
        QLineEdit *edit;
};

#endif
