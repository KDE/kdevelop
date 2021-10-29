/*
    SPDX-FileCopyrightText: 2002 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SAVEDIALOG_H
#define KDEVPLATFORM_SAVEDIALOG_H

#include <QDialog>

class QListWidget;

namespace KDevelop {

class IDocument;

class KSaveSelectDialog : public QDialog
{
    Q_OBJECT

public:
    KSaveSelectDialog( const QList<IDocument*>& files, QWidget * parent );
    ~KSaveSelectDialog() override;

private Q_SLOTS:
    void save();

private:
    QListWidget* m_listWidget;
};

}

#endif
