/*
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    Adapted for Git
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    Pimpl-ed and exported
    SPDX-FileCopyrightText: 2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_DVCSIMPORTMETADATAWIDGET_H
#define KDEVPLATFORM_DVCSIMPORTMETADATAWIDGET_H

#include <vcs/widgets/vcsimportmetadatawidget.h>

namespace Ui {
class DvcsImportMetadataWidget;
}

class DvcsImportMetadataWidgetPrivate;

/**
 * Asks the user for all options needed to import an existing directory into
 * a Git repository
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class KDEVPLATFORMVCS_EXPORT DvcsImportMetadataWidget : public KDevelop::VcsImportMetadataWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DvcsImportMetadataWidget)
    
public:
    explicit DvcsImportMetadataWidget(QWidget* parent=nullptr);
    ~DvcsImportMetadataWidget() override;

    QUrl source() const override;
    KDevelop::VcsLocation destination() const override;
    QString message() const override; //Is not used, it returns an empty string
    void setSourceLocation( const KDevelop::VcsLocation& ) override;
    void setSourceLocationEditable( bool ) override;
    void setMessage(const QString& message) override;
    bool hasValidData() const override;

private:
    DvcsImportMetadataWidgetPrivate *const d_ptr;
};

#endif
