/***************************************************************************
 *   Copyright 2009 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#ifndef KDEVPLATFORM_MAIN_H
#define KDEVPLATFORM_MAIN_H

#include <KDE/KDialog>
#include <KDE/KDebug>
#include <KDE/KLocalizedString>

#include <QtCore/QDir>
#include <QtGui>   

class AuthForm : public QWidget
{
    Q_OBJECT
public:
    /** Auth types */
    enum AuthType
    {
        PASSWORD = 1, /**< Password Auth */
        PUBKEY = 2, /**< Auth using publick key */
        BOTH = 3, /**< Both PASSWORD and PUBKEY */
    };
    
    AuthForm(const QString& _username = "", uint _auth_type_mask = PUBKEY, QWidget* parent = 0)
           : QWidget(parent) 
    {
        username = new QLineEdit(_username);
        username_label = new QLabel(i18n("&Username:"));
        username_label->setBuddy(username);
        
        password_or_passphrase = new QLineEdit();
        password_or_passphrase->setEchoMode(QLineEdit::Password);
        password_or_passphrase_label = new QLabel(i18n("&Password or passphrase:"));
        password_or_passphrase_label->setBuddy(password_or_passphrase);

        pub_key = new QLineEdit(QDir::homePath() + "/.ssh/id_rsa.pub");
        pub_key_label = new QLabel(i18n("Publi&c key:"));
        pub_key_label->setBuddy(pub_key);

        private_key = new QLineEdit(QDir::homePath() + "/.ssh/id_rsa");
        private_key_label = new QLabel(i18n("P&rivate key:"));
        private_key_label->setBuddy(private_key);
        
        select_auth_type = new QComboBox();
        if (_auth_type_mask & PASSWORD)
            select_auth_type->addItem(i18n("Password auth"), QVariant(PASSWORD));
        if (_auth_type_mask & PUBKEY)
            select_auth_type->addItem(i18n("Public key auth"), QVariant(PUBKEY));
        if (_auth_type_mask == 0)
            select_auth_type->addItem(i18n("No auth type supported by server"), QVariant(0));
        
        mgrid = new QGridLayout();
        mgrid->addWidget(username_label, 0,0);
        mgrid->addWidget(username, 0,1);
        mgrid->addWidget(password_or_passphrase_label, 1,0);
        mgrid->addWidget(password_or_passphrase, 1,1);
        
        keys_grid = new QGridLayout();
        keys_grid->addWidget(pub_key_label, 0,0);
        keys_grid->addWidget(pub_key, 0,1);
        keys_grid->addWidget(private_key_label, 1,0);
        keys_grid->addWidget(private_key, 1,1);
        
        fileBox = new QGroupBox(i18n("Select keys"));
        fileBox->setLayout(keys_grid);
        
        mlayout = new QVBoxLayout();
        mlayout->addWidget(select_auth_type);
        mlayout->addLayout(mgrid);
        mlayout->addWidget(fileBox);
        setLayout(mlayout);
        
        change_visibility(select_auth_type->currentIndex());
        password_or_passphrase->setFocus();
        QObject::connect(select_auth_type, SIGNAL(currentIndexChanged(int)),
                                           SLOT(change_visibility(const int)));
    }
    
    QLineEdit *username;
    QLineEdit *password_or_passphrase;
    QLineEdit *pub_key;
    QLineEdit *private_key;
    QComboBox *select_auth_type;

private slots:
    void change_visibility(const int index)
    {
    kDebug() << "index:" << index;
        if (select_auth_type->itemData(index) == QVariant(PUBKEY))
            fileBox->show();
        else
            fileBox->hide();
    }

private:
    QLabel *username_label;
    QLabel *password_or_passphrase_label;
    QLabel *pub_key_label;
    QLabel *private_key_label;
    QGridLayout *mgrid;
    QGridLayout *keys_grid;
    QGroupBox *fileBox;
    QVBoxLayout *mlayout;
    
};


#endif
