/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SELECTCOREDIALOG_H
#define SELECTCOREDIALOG_H

#include <QDialog>
#include <QUrl>

#include "ui_selectcoredialog.h"

namespace KDevMI {

class SelectCoreDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectCoreDialog(QWidget *parent = nullptr);
    QUrl executableFile() const;
    QUrl core() const;

private:
    void validateSelection();

    Ui::SelectCoreDialog m_ui;
};

} // end of namespace KDevMI

#endif
