/*
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    Adapted for Git
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    Pimpl-ed and exported
    SPDX-FileCopyrightText: 2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "dvcsimportmetadatawidget.h"

#include <vcslocation.h>
#include "ui_dvcsimportmetadatawidget.h"

class DvcsImportMetadataWidgetPrivate
{
    friend class DvcsImportMetadataWidget;

    explicit DvcsImportMetadataWidgetPrivate(Ui::DvcsImportMetadataWidget* ui) : m_ui(ui) {}
    ~DvcsImportMetadataWidgetPrivate() { delete m_ui; }

    Ui::DvcsImportMetadataWidget* m_ui;
};

DvcsImportMetadataWidget::DvcsImportMetadataWidget(QWidget *parent)
    : KDevelop::VcsImportMetadataWidget(parent),
    d_ptr(new DvcsImportMetadataWidgetPrivate(new Ui::DvcsImportMetadataWidget))
{
    Q_D(DvcsImportMetadataWidget);
    d->m_ui->setupUi(this);

    d->m_ui->sourceLoc->setEnabled( false );
    d->m_ui->sourceLoc->setMode( KFile::Directory );
    connect( d->m_ui->sourceLoc, &KUrlRequester::textChanged, this, &DvcsImportMetadataWidget::changed );
    connect( d->m_ui->sourceLoc, &KUrlRequester::urlSelected, this, &DvcsImportMetadataWidget::changed );
    connect(d->m_ui->message, &QTextEdit::textChanged, this, &DvcsImportMetadataWidget::changed);
}

DvcsImportMetadataWidget::~DvcsImportMetadataWidget()
{
    delete d_ptr;
}

QUrl DvcsImportMetadataWidget::source() const
{
    Q_D(const DvcsImportMetadataWidget);
    return d->m_ui->sourceLoc->url();
}

KDevelop::VcsLocation DvcsImportMetadataWidget::destination() const
{
    // Used for compatibility with import
    Q_D(const DvcsImportMetadataWidget);
    KDevelop::VcsLocation dest;
    dest.setRepositoryServer(d->m_ui->sourceLoc->url().url());
    return dest;
}

QString DvcsImportMetadataWidget::message( ) const
{
    Q_D(const DvcsImportMetadataWidget);
    return d->m_ui->message->toPlainText();
}

void DvcsImportMetadataWidget::setSourceLocation( const KDevelop::VcsLocation& url )
{
    Q_D(const DvcsImportMetadataWidget);
    d->m_ui->sourceLoc->setUrl( url.localUrl() );
}

void DvcsImportMetadataWidget::setSourceLocationEditable( bool enable )
{
    Q_D(const DvcsImportMetadataWidget);
    d->m_ui->sourceLoc->setEnabled( enable );
}

void DvcsImportMetadataWidget::setMessage(const QString& message)
{
    Q_D(DvcsImportMetadataWidget);
    d->m_ui->message->setText(message);
}

bool DvcsImportMetadataWidget::hasValidData() const
{
    Q_D(const DvcsImportMetadataWidget);
    return !d->m_ui->message->toPlainText().isEmpty() && !d->m_ui->sourceLoc->text().isEmpty();
}

#include "moc_dvcsimportmetadatawidget.cpp"
