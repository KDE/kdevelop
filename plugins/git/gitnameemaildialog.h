 /**************************************************************************
 *   Copyright 2016 Artur Puzio <cytadela88@gmail.com>                     *
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

#ifndef KDEVPLATFORM_PLUGIN_GIT_NAMEEMAILDIALOG_H
#define KDEVPLATFORM_PLUGIN_GIT_NAMEEMAILDIALOG_H

#include <QtWidgets>
#include <QDialog>


class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
namespace Ui { class GitNameEmailDialog; }

class GitNameEmailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GitNameEmailDialog(QWidget *parent = 0);
    ~GitNameEmailDialog() override;
    void setName(const QString& name);
    void setEmail(const QString& email);
    QString name() const;
    QString email() const;
    bool isGlobal() const;

private slots:
    void updateUi();
private:
    QScopedPointer<Ui::GitNameEmailDialog> ui;
};

#endif //KDEVPLATFORM_PLUGIN_GIT_NAMEEMAILDIALOG_H
