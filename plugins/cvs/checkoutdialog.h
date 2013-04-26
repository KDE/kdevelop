/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CHECKOUTDIALOG_H
#define KDEVPLATFORM_PLUGIN_CHECKOUTDIALOG_H

#include <QDialog>

#include <KJob>
#include <KUrl>

#include "ui_checkoutdialog.h"

class CvsPlugin;

/**
 * Allows the user to define from where to checkout
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CheckoutDialog : public KDialog, private Ui::CheckoutDialogBase
{
    Q_OBJECT
public:
    explicit CheckoutDialog(CvsPlugin* plugin, QWidget *parent=0);
    virtual ~CheckoutDialog();

public slots:
    virtual void accept();
    void jobFinished(KJob* job);

private:
    CvsPlugin* m_plugin;
};

#endif
