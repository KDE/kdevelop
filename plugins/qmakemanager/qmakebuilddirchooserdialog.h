/*
    SPDX-FileCopyrightText: 2011 Martin Heide <martin.heide@gmx.net>
    SPDX-FileCopyrightText: 2011 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKEBUILDDIRCHOOSERDIALOG_H
#define QMAKEBUILDDIRCHOOSERDIALOG_H

#include <QDialog>

class QMakeBuildDirChooser;

class QDialogButtonBox;

namespace KDevelop {
class IProject;
}

class QMakeBuildDirChooserDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit QMakeBuildDirChooserDialog(KDevelop::IProject* project, QWidget *parent = nullptr);
    ~QMakeBuildDirChooserDialog() override;

    void loadConfig();
    void saveConfig();

    QString buildDir() const;

public Q_SLOTS:
    void accept() override;

private Q_SLOTS:
    void validate();

private:
    QMakeBuildDirChooser* m_chooserUi;
    QDialogButtonBox* m_buttonBox;
};

#endif
