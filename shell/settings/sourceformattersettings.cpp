/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/
#include "sourceformattersettings.h"

#include <QVBoxLayout>
#include <QList>
#include <QListWidgetItem>
#include <QInputDialog>
#include <QMimeDatabase>
#include <QDebug>
#include <KMessageBox>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/configinterface.h>
#include <KCoreAddons/KAboutData>
#include <KI18n/KLocalizedString>
#include <KConfigGroup>

#include <interfaces/iplugin.h>
#include <interfaces/ilanguage.h>
#include <interfaces/isourceformatter.h>
#include <shell/core.h>
#include <shell/plugincontroller.h>
#include <shell/languagecontroller.h>

#include "editstyledialog.h"
#include "../debug.h"

#define STYLE_ROLE (Qt::UserRole+1)

using KDevelop::Core;
using KDevelop::ISourceFormatter;
using KDevelop::SourceFormatterStyle;
using KDevelop::SourceFormatterController;
using KDevelop::SourceFormatter;

const QString SourceFormatterSettings::userStylePrefix( "User" );

LanguageSettings::LanguageSettings()
    : selectedFormatter(0), selectedStyle(0) {
}

SourceFormatterSettings::SourceFormatterSettings(QWidget* parent)
    : KDevelop::ConfigPage(nullptr, nullptr, parent)
{
    setupUi(this);
    connect( cbLanguages, static_cast<void(KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &SourceFormatterSettings::selectLanguage );
    connect( cbFormatters, static_cast<void(KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &SourceFormatterSettings::selectFormatter );
    connect( chkKateModelines, &QCheckBox::toggled, this, &SourceFormatterSettings::somethingChanged );
    connect( chkKateOverrideIndentation, &QCheckBox::toggled, this, &SourceFormatterSettings::somethingChanged );
    connect( styleList, &QListWidget::currentRowChanged, this, &SourceFormatterSettings::selectStyle );
    connect( btnDelStyle, &QPushButton::clicked, this, &SourceFormatterSettings::deleteStyle );
    connect( btnNewStyle, &QPushButton::clicked, this, &SourceFormatterSettings::newStyle );
    connect( btnEditStyle, &QPushButton::clicked, this, &SourceFormatterSettings::editStyle );
    connect( styleList, &QListWidget::itemChanged, this, &SourceFormatterSettings::styleNameChanged );

    m_document = KTextEditor::Editor::instance()->createDocument(this);
    m_document->setReadWrite(false);

    m_view = m_document->createView(textEditor);
    m_view->setStatusBarEnabled(false);

    QVBoxLayout *layout2 = new QVBoxLayout(textEditor);
    layout2->addWidget(m_view);
    textEditor->setLayout(layout2);
    m_view->show();

    KTextEditor::ConfigInterface *iface =
    qobject_cast<KTextEditor::ConfigInterface*>(m_view);
    if (iface) {
        iface->setConfigValue("dynamic-word-wrap", false);
        iface->setConfigValue("icon-bar", false);
    }
}

SourceFormatterSettings::~SourceFormatterSettings()
{
    qDeleteAll(formatters);
}

void selectAvailableStyle(LanguageSettings& lang) {
    Q_ASSERT(!lang.selectedFormatter->styles.empty());
    lang.selectedStyle = *lang.selectedFormatter->styles.begin();
}

void SourceFormatterSettings::reset()
{
    SourceFormatterController* fmtctrl = Core::self()->sourceFormatterControllerInternal();
    foreach( KDevelop::IPlugin* plugin, KDevelop::ICore::self()->pluginController()->allPluginsForExtension( "org.kdevelop.ISourceFormatter" ) )
    {
        KDevelop::ISourceFormatter* ifmt = plugin->extension<ISourceFormatter>();
        KPluginInfo info = KDevelop::Core::self()->pluginControllerInternal()->pluginInfo( plugin );
        KDevelop::SourceFormatter* formatter;
        FormatterMap::const_iterator iter = formatters.constFind(ifmt->name());
        if (iter == formatters.constEnd()) {
            formatter = fmtctrl->createFormatterForPlugin(ifmt);
            formatters[ifmt->name()] = formatter;
        } else {
            formatter = iter.value();
        }
        for( const SourceFormatterStyle* style: formatter->styles ) {
            for ( const SourceFormatterStyle::MimeHighlightPair& item: style->mimeTypes() ) {
                QMimeType mime = QMimeDatabase().mimeTypeForName(item.mimeType);
                if (!mime.isValid()) {
                    qWarning() << "plugin" << info.name() << "supports unknown mimetype entry" << item.mimeType;
                    continue;
                }
                QString languageName = item.highlightMode;
                LanguageSettings& l = languages[languageName];
                l.mimetypes.append(mime);
                l.formatters.insert( formatter );
            }
        }
    }

    // Sort the languages, preferring firstly active, then loaded languages
    QList<QString> sortedLanguages;

    foreach( KDevelop::ILanguage* language,
                KDevelop::ICore::self()->languageController()->activeLanguages() +
                KDevelop::ICore::self()->languageController()->loadedLanguages() )
    {
        if( languages.contains( language->name() ) && !sortedLanguages.contains(language->name()) ) {
            sortedLanguages.push_back( language->name() );
        }
    }

    foreach( const QString& name, languages.keys() )
        if( !sortedLanguages.contains( name ) )
            sortedLanguages.push_back( name );

    foreach( const QString& name, sortedLanguages )
    {
        // Pick the first appropriate mimetype for this language
        KConfigGroup grp = fmtctrl->configuration();
        LanguageSettings& l = languages[name];
        foreach (const QMimeType& mimetype, l.mimetypes) {
            QStringList formatterAndStyleName = grp.readEntry(mimetype.name(), "").split("||", QString::KeepEmptyParts);
            FormatterMap::const_iterator formatterIter = formatters.constFind(formatterAndStyleName.first());
            if (formatterIter == formatters.constEnd()) {
                qCDebug(SHELL) << "Reference to unknown formatter" << formatterAndStyleName.first();
                Q_ASSERT(!l.formatters.empty());        // otherwise there should be no entry for 'name'
                l.selectedFormatter = *l.formatters.begin();
                selectAvailableStyle(l);
            } else {
                l.selectedFormatter = formatterIter.value();
                SourceFormatter::StyleMap::const_iterator styleIter = l.selectedFormatter->styles.constFind(formatterAndStyleName.at( 1 ));
                if (styleIter == l.selectedFormatter->styles.constEnd()) {
                    qCDebug(SHELL) << "No style" << formatterAndStyleName.at( 1 ) << "found for formatter" << formatterAndStyleName.first();
                    selectAvailableStyle(l);
                } else {
                    l.selectedStyle = styleIter.value();
                }
            }
            break;
        }
        if (!l.selectedFormatter) {
            Q_ASSERT(!l.formatters.empty());
            l.selectedFormatter = *l.formatters.begin();
        }
        if (!l.selectedStyle) {
            selectAvailableStyle(l);
        }
    }
    bool b = blockSignals( true );
    cbLanguages->blockSignals( !b );
    cbFormatters->blockSignals( !b );
    styleList->blockSignals( !b );
    chkKateModelines->blockSignals( !b );
    chkKateOverrideIndentation->blockSignals( !b );
    cbLanguages->clear();
    cbFormatters->clear();
    styleList->clear();
    chkKateModelines->setChecked( fmtctrl->configuration().readEntry( SourceFormatterController::kateModeLineConfigKey, false ) );
    chkKateOverrideIndentation->setChecked( fmtctrl->configuration().readEntry( SourceFormatterController::kateOverrideIndentationConfigKey, false ) );
    foreach( const QString& name, sortedLanguages )
    {
        cbLanguages->addItem( name );
    }
    if( cbLanguages->count() == 0 )
    {
        cbLanguages->setEnabled( false );
        selectLanguage( -1 );
    } else
    {
        cbLanguages->setCurrentIndex( 0 );
        selectLanguage( 0 );
    }
    updatePreview();
    blockSignals( b );
    cbLanguages->blockSignals( b );
    cbFormatters->blockSignals( b );
    styleList->blockSignals( b );
    chkKateModelines->blockSignals( b );
    chkKateOverrideIndentation->blockSignals( b );
}

void SourceFormatterSettings::apply()
{
    KConfigGroup grp = Core::self()->sourceFormatterControllerInternal()->configuration();

    for ( LanguageMap::const_iterator iter = languages.constBegin(); iter != languages.constEnd(); ++iter ) {
        foreach(const QMimeType& mime, iter.value().mimetypes) {
            grp.writeEntry(mime.name(), QString("%1||%2").arg(iter.value().selectedFormatter->formatter->name()).arg(iter.value().selectedStyle->name()));
        }
    }
    foreach( SourceFormatter* fmt, formatters )
    {
        KConfigGroup fmtgrp = grp.group( fmt->formatter->name() );

        // delete all styles so we don't leave any behind when all user styles are deleted
        foreach( const QString& subgrp, fmtgrp.groupList() )
        {
            if( subgrp.startsWith( userStylePrefix ) ) {
                fmtgrp.deleteGroup( subgrp );
            }
        }
        foreach( const SourceFormatterStyle* style, fmt->styles )
        {
            if( style->name().startsWith( userStylePrefix ) )
            {
                KConfigGroup stylegrp = fmtgrp.group( style->name() );
                stylegrp.writeEntry( SourceFormatterController::styleCaptionKey, style->caption() );
                stylegrp.writeEntry( SourceFormatterController::styleContentKey, style->content() );
                stylegrp.writeEntry( SourceFormatterController::styleMimeTypesKey, style->mimeTypesVariant() );
                stylegrp.writeEntry( SourceFormatterController::styleSampleKey, style->overrideSample() );
            }
        }
    }
    grp.writeEntry( SourceFormatterController::kateModeLineConfigKey, chkKateModelines->isChecked() );
    grp.writeEntry( SourceFormatterController::kateOverrideIndentationConfigKey, chkKateOverrideIndentation->isChecked() );

    grp.sync();

    Core::self()->sourceFormatterControllerInternal()->settingsChanged();
}

void SourceFormatterSettings::enableStyleButtons()
{
    bool userEntry = styleList->currentItem()
                     && styleList->currentItem()->data( STYLE_ROLE ).toString().startsWith( userStylePrefix );

    QString languageName = cbLanguages->currentText();
    QMap< QString, LanguageSettings >::const_iterator it = languages.constFind(languageName);
    bool hasEditWidget = false;
    if (it != languages.constEnd()) {
        const LanguageSettings& l = it.value();
        Q_ASSERT(l.selectedFormatter);
        ISourceFormatter* fmt = l.selectedFormatter->formatter;
        hasEditWidget = ( fmt && fmt->editStyleWidget( l.mimetypes.first() ) );
    }
    btnDelStyle->setEnabled( userEntry );
    btnEditStyle->setEnabled( userEntry && hasEditWidget );
    btnNewStyle->setEnabled( cbFormatters->currentIndex() >= 0 && hasEditWidget );
}

void SourceFormatterSettings::selectLanguage( int idx )
{
    cbFormatters->clear();
    if( idx < 0 )
    {
        cbFormatters->setEnabled( false );
        selectFormatter( -1 );
        return;
    }
    cbFormatters->setEnabled( true );
    bool b = cbFormatters->blockSignals( true );
    LanguageSettings& l = languages[cbLanguages->itemText( idx )];
    foreach( const SourceFormatter* fmt, l.formatters )
    {
        cbFormatters->addItem( fmt->formatter->caption(), fmt->formatter->name() );
    }
    cbFormatters->setCurrentIndex(cbFormatters->findData(l.selectedFormatter->formatter->name()));
    cbFormatters->blockSignals(b);
    selectFormatter( cbFormatters->currentIndex() );
    emit changed();
}

void SourceFormatterSettings::selectFormatter( int idx )
{
    styleList->clear();
    if( idx < 0 )
    {
        styleList->setEnabled( false );
        enableStyleButtons();
        return;
    }
    styleList->setEnabled( true );
    LanguageSettings& l = languages[ cbLanguages->currentText() ];
    Q_ASSERT( idx < l.formatters.size() );
    FormatterMap::const_iterator formatterIter = formatters.constFind(cbFormatters->itemData( idx ).toString());
    Q_ASSERT( formatterIter != formatters.constEnd() );
    Q_ASSERT( l.formatters.contains(formatterIter.value()) );
    if (l.selectedFormatter != formatterIter.value()) {
        l.selectedFormatter = formatterIter.value();
        l.selectedStyle = 0;    // will hold 0 until a style is picked
    }
    foreach( const SourceFormatterStyle* style, formatterIter.value()->styles ) {
        if ( ! style->supportsLanguage(cbLanguages->currentText())) {
            // do not list items which do not support the selected language
            continue;
        }
        QListWidgetItem* item = addStyle( *style );
        if (style == l.selectedStyle) {
            styleList->setCurrentItem(item);
        }
    }
    if (l.selectedStyle == 0) {
        styleList->setCurrentRow(0);
    }
    enableStyleButtons();
    emit changed();
}

void SourceFormatterSettings::selectStyle( int row )
{
    if( row < 0 )
    {
        enableStyleButtons();
        return;
    }
    styleList->setCurrentRow( row );
    LanguageSettings& l = languages[ cbLanguages->currentText() ];
    l.selectedStyle = l.selectedFormatter->styles[styleList->item( row )->data( STYLE_ROLE ).toString()];
    enableStyleButtons();
    updatePreview();
    emit changed();
}

void SourceFormatterSettings::deleteStyle()
{
    Q_ASSERT( styleList->currentRow() >= 0 );

    QListWidgetItem* item = styleList->currentItem();

    LanguageSettings& l = languages[ cbLanguages->currentText() ];
    SourceFormatter* fmt = l.selectedFormatter;
    SourceFormatter::StyleMap::iterator styleIter = fmt->styles.find(item->data( STYLE_ROLE ).toString());
    QStringList otherLanguageNames;
    QList<LanguageSettings*> otherlanguages;
    for ( LanguageMap::iterator languageIter = languages.begin(); languageIter != languages.end(); ++languageIter ) {
        if ( &languageIter.value() != &l && languageIter.value().selectedStyle == styleIter.value() ) {
            otherLanguageNames.append(languageIter.key());
            otherlanguages.append(&languageIter.value());
        }
    }
    if (!otherLanguageNames.empty() &&
        KMessageBox::warningContinueCancel(this,
        i18n("The style %1 is also used for the following languages:\n%2.\nAre you sure you want to delete it?",
        styleIter.value()->caption(), otherLanguageNames.join("\n")), i18n("Style being deleted")) != KMessageBox::Continue) {
        return;
    }
    styleList->takeItem( styleList->currentRow() );
    fmt->styles.erase(styleIter);
    delete item;
    selectStyle( styleList->count() > 0 ? 0 : -1 );
    foreach (LanguageSettings* lang, otherlanguages) {
        selectAvailableStyle(*lang);
    }
    updatePreview();
    emit changed();
}

void SourceFormatterSettings::editStyle()
{
    QString language = cbLanguages->currentText();
    Q_ASSERT( languages.contains( language ) );
    LanguageSettings& l = languages[ language ];
    SourceFormatter* fmt = l.selectedFormatter;

    QMimeType mimetype = l.mimetypes.first();
    if( fmt->formatter->editStyleWidget( mimetype ) != 0 ) {
        EditStyleDialog dlg( fmt->formatter, mimetype, *l.selectedStyle, this );
        if( dlg.exec() == QDialog::Accepted )
        {
            l.selectedStyle->setContent(dlg.content());
        }
        updatePreview();
        emit changed();
    }
}

void SourceFormatterSettings::newStyle()
{
    QListWidgetItem* item = styleList->currentItem();
    LanguageSettings& l = languages[ cbLanguages->currentText() ];
    SourceFormatter* fmt = l.selectedFormatter;
    int idx = 0;
    for( int i = 0; i < styleList->count(); i++ )
    {
        QString name = styleList->item( i )->data( STYLE_ROLE ).toString();
        if( name.startsWith( userStylePrefix ) && name.mid( userStylePrefix.length() ).toInt() >= idx )
        {
            idx = name.mid( userStylePrefix.length() ).toInt();
        }
    }
    // Increase number for next style
    idx++;
    SourceFormatterStyle* s = new SourceFormatterStyle( QString( "%1%2" ).arg( userStylePrefix ).arg( idx ) );
    if( item ) {
        SourceFormatterStyle* existstyle = fmt->styles[ item->data( STYLE_ROLE ).toString() ];
        s->setCaption( i18n( "New %1", existstyle->caption() ) );
        s->copyDataFrom( existstyle );
    } else {
        s->setCaption( i18n( "New Style" ) );
    }
    fmt->styles[ s->name() ] = s;
    QListWidgetItem* newitem = addStyle( *s );
    selectStyle( styleList->row( newitem ) );
    styleList->editItem( newitem );
    emit changed();
}

void SourceFormatterSettings::styleNameChanged( QListWidgetItem* item )
{
    if ( !item->isSelected() ) {
        return;
    }

    LanguageSettings& l = languages[ cbLanguages->currentText() ];
    l.selectedStyle->setCaption( item->text() );
    emit changed();
}

QListWidgetItem* SourceFormatterSettings::addStyle( const SourceFormatterStyle& s )
{
    QListWidgetItem* item = new QListWidgetItem( styleList );
    item->setText( s.caption() );
    item->setData( STYLE_ROLE, s.name() );
    if( s.name().startsWith( userStylePrefix ) )
    {
        item->setFlags( item->flags() | Qt::ItemIsEditable );
    }
    styleList->addItem( item );
    return item;
}

void SourceFormatterSettings::updatePreview()
{
    m_document->setReadWrite( true );

    QString langName = cbLanguages->itemText( cbLanguages->currentIndex() );
    if( !langName.isEmpty() )
    {
        LanguageSettings& l = languages[ langName ];
        SourceFormatter* fmt = l.selectedFormatter;
        SourceFormatterStyle* style = l.selectedStyle;

        descriptionLabel->setText( style->description() );
        if( style->description().isEmpty() )
            descriptionLabel->hide();
        else
            descriptionLabel->show();

        if( style->usePreview() )
        {
            ISourceFormatter* ifmt = fmt->formatter;
            QMimeType mime = l.mimetypes.first();
            m_document->setHighlightingMode( style->modeForMimetype( mime ) );

            //NOTE: this is ugly, but otherwise kate might remove tabs again :-/
            // see also: https://bugs.kde.org/show_bug.cgi?id=291074
            KTextEditor::ConfigInterface* iface = qobject_cast<KTextEditor::ConfigInterface*>(m_document);
            QVariant oldReplaceTabs;
            if (iface) {
                oldReplaceTabs = iface->configValue("replace-tabs");
                iface->setConfigValue("replace-tabs", false);
            }

            m_document->setText( ifmt->formatSourceWithStyle( *style, ifmt->previewText( *style, mime ), QUrl(), mime ) );

            if (iface) {
                iface->setConfigValue("replace-tabs", oldReplaceTabs);
            }

            previewLabel->show();
            textEditor->show();
        }else{
            previewLabel->hide();
            textEditor->hide();
        }
    } else
    {
        m_document->setText( i18n( "No Language selected" ) );
    }
    m_view->setCursorPosition( KTextEditor::Cursor( 0, 0 ) );
    m_document->setReadWrite( false );
}

void SourceFormatterSettings::somethingChanged()
{
    // Widgets are managed manually, so we have to explicitly tell KCModule
    // that we have some changes, otherwise it won't call "save" and/or will not activate
    // "Appy"
    emit changed();
}

QString SourceFormatterSettings::name() const
{
    return i18n("Source Formatter");
}

QString SourceFormatterSettings::fullName() const
{
    return i18n("Configure Source Formatter");
}

QIcon SourceFormatterSettings::icon() const
{
    return QIcon::fromTheme(QStringLiteral("text-field"));
}


#include "sourceformattersettings.moc"
