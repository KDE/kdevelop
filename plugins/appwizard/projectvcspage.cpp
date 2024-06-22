/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectvcspage.h"
#include "ui_projectvcspage.h"

#include <QStackedWidget>

#include <KComboBox>
#include <KLocalizedString>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>

#include <vcs/vcslocation.h>
#include <vcs/widgets/vcsimportmetadatawidget.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

using namespace KDevelop;

ProjectVcsPage::ProjectVcsPage( KDevelop::IPluginController* controller, QWidget * parent )
    : AppWizardPageWidget(parent)
    , m_currentImportWidget(nullptr)
    , m_ui(new Ui::ProjectVcsPage)
{
    m_ui->setupUi( this );
    const QList<KDevelop::IPlugin*> vcsplugins = controller->allPluginsForExtension(QStringLiteral("org.kdevelop.IBasicVersionControl"));
    int idx = 1;
    m_ui->vcsImportOptions->insertWidget( 0, new QWidget(this) );
    m_ui->vcsTypes->insertItem( 0, i18nc("@item:inlistbox No Version Control Support chosen", "None") );
    for (KDevelop::IPlugin* plugin : vcsplugins) {
        auto* iface = plugin->extension<KDevelop::IBasicVersionControl>();
        if( iface  )
        {
            KDevelop::VcsImportMetadataWidget* widget = iface->createImportMetadataWidget(
                                                    m_ui->vcsImportOptions );
            if( widget )
            {
                // untranslated on purpose, as English might be lingua franca at most target users
                // perhaps make default string configurable if people request it
                widget->setMessage(QStringLiteral("Initial import"));
                widget->setSourceLocationEditable( false );
                widget->setUseSourceDirForDestination( true );
                m_ui->vcsTypes->insertItem( idx, iface->name() );
                importWidgets.push_back( widget );
                vcsPlugins.push_back( qMakePair( controller->pluginInfo( plugin ).pluginId(), iface->name() ) );
                m_ui->vcsImportOptions->insertWidget( idx, widget );
                idx++;
            }
        }
    }
    connect( m_ui->vcsTypes, QOverload<int>::of(&KComboBox::activated),
             m_ui->vcsImportOptions, &QStackedWidget::setCurrentIndex );
    connect( m_ui->vcsTypes, QOverload<int>::of(&KComboBox::activated),
             this, &ProjectVcsPage::vcsTypeChanged );
    vcsTypeChanged(m_ui->vcsTypes->currentIndex());
}


void ProjectVcsPage::vcsTypeChanged( int idx )
{
    if (m_currentImportWidget) {
        disconnect(m_currentImportWidget, &VcsImportMetadataWidget::changed, this, &ProjectVcsPage::validateData);
    }

    // first type in list is "no vcs", without an import widget
    const int widgetIndex = idx - 1;
    m_currentImportWidget = importWidgets.value(widgetIndex);

    validateData();

    if (m_currentImportWidget) {
        connect(m_currentImportWidget, &VcsImportMetadataWidget::changed, this, &ProjectVcsPage::validateData);
    }
}

void ProjectVcsPage::validateData()
{
    if( shouldContinue() ) {
        emit valid();
    } else {
        emit invalid();
    }
}


ProjectVcsPage::~ProjectVcsPage( )
{
    delete m_ui;
}

void ProjectVcsPage::setSourceLocation( const QUrl& s )
{
    for (KDevelop::VcsImportMetadataWidget* widget : std::as_const(importWidgets)) {
        widget->setSourceLocation( KDevelop::VcsLocation( s ) );
    }
}

QString ProjectVcsPage::pluginName() const
{
    int idx = m_ui->vcsTypes->currentIndex() - 1;
    if ( idx < 0 || idx >= vcsPlugins.size())
    return QString();

    // FIXME: Two return statements
    return vcsPlugins[idx].first;
}


QString ProjectVcsPage::commitMessage() const
{
    if (!m_currentImportWidget) {
        return QString();
    }

    return m_currentImportWidget->message();
}

QUrl ProjectVcsPage::source() const
{
    if (!m_currentImportWidget) {
        return QUrl();
    }

    return m_currentImportWidget->source();
}

KDevelop::VcsLocation ProjectVcsPage::destination() const
{
    if (!m_currentImportWidget) {
        return KDevelop::VcsLocation();
    }

    return m_currentImportWidget->destination();
}


bool ProjectVcsPage::shouldContinue()
{
    if (!m_currentImportWidget) {
        return true;
    }

    return m_currentImportWidget->hasValidData();
}

#include "moc_projectvcspage.cpp"
