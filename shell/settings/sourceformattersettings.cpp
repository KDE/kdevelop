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
#include <KMessageBox>
#include <KIconLoader>
#include <KGenericFactory>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/configinterface.h>

#include <interfaces/iplugin.h>
#include <shell/core.h>
#include <interfaces/isourceformatter.h>
#include <shell/sourceformattercontroller.h>
#include <shell/plugincontroller.h>

#include "editstyledialog.h"

#define STYLE_ROLE (Qt::UserRole+1)

K_PLUGIN_FACTORY(SourceFormatterSettingsFactory, registerPlugin<SourceFormatterSettings>();)
K_EXPORT_PLUGIN(SourceFormatterSettingsFactory("kcm_kdevsourceformattersettings"))

using KDevelop::Core;
using KDevelop::ISourceFormatter;
using KDevelop::SourceFormatterStyle;
using KDevelop::SourceFormatterController;

const QString SourceFormatterSettings::userStylePrefix( "User" );

SourceFormatterSettings::SourceFormatterSettings(QWidget *parent, const QVariantList &args)
    : KCModule(SourceFormatterSettingsFactory::componentData(), parent, args)
{
    setupUi(this);
    connect( cbLanguages, SIGNAL(currentIndexChanged(int)), SLOT(selectLanguage(int)) );
    connect( cbFormatters, SIGNAL(currentIndexChanged(int)), SLOT(selectFormatter(int)) );
    connect( chkKateModelines, SIGNAL(toggled(bool)), SIGNAL(changed(bool)) );
    connect( styleList, SIGNAL(currentRowChanged(int)), SLOT(selectStyle(int)) );
    connect( btnDelStyle, SIGNAL(clicked()), SLOT(deleteStyle()) );
    connect( btnNewStyle, SIGNAL(clicked()), SLOT(newStyle()) );
    connect( btnEditStyle, SIGNAL(clicked()), SLOT(editStyle()) );
    connect( styleList, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(styleNameChanged(QListWidgetItem*)) );

    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor)
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found.\n"
        "Please check your KDE installation."));

    m_document = editor->createDocument(this);
    m_document->setReadWrite(false);

    KTextEditor::View* view = qobject_cast<KTextEditor::View*>(m_document->createView(textEditor));
    QVBoxLayout *layout2 = new QVBoxLayout(textEditor);
    layout2->addWidget(view);
    textEditor->setLayout(layout2);
    view->show();

    KTextEditor::ConfigInterface *iface =
    qobject_cast<KTextEditor::ConfigInterface*>(view);
    if (iface) {
        iface->setConfigValue("dynamic-word-wrap", false);
        iface->setConfigValue("icon-bar", false);
    }
}

SourceFormatterSettings::~SourceFormatterSettings()
{
}

void SourceFormatterSettings::load()
{
    SourceFormatterController* fmtctrl = Core::self()->sourceFormatterControllerInternal();
    foreach( KDevelop::IPlugin* plugin, KDevelop::ICore::self()->pluginController()->allPluginsForExtension( "org.kdevelop.ISourceFormatter" ) )
    {
        KDevelop::ISourceFormatter* ifmt = plugin->extension<ISourceFormatter>();
        KPluginInfo info = KDevelop::Core::self()->pluginControllerInternal()->pluginInfo( plugin );
        foreach( const QString& mime, info.property( SourceFormatterController::supportedMimeTypesKey ).toStringList() )
        {
            SourceFormatterLanguage l;
            QMap<QString,SourceFormatterLanguage>::iterator it = languages.find( mime );
            if( it != languages.end() )
            {
                l = it.value();
            }
            l.mimeType = mime;
            SourceFormatter sfmt;

            Q_ASSERT( !l.formatters.contains( ifmt->name() ) );

            sfmt.formatter = ifmt;

            foreach( const KDevelop::SourceFormatterStyle& style, ifmt->predefinedStyles() )
            {
                sfmt.styles[ style.name() ] = style;
            }
            KConfigGroup grp = fmtctrl->configuration();
            if( grp.hasGroup( ifmt->name() ) )
            {
                KConfigGroup fmtgrp = grp.group( ifmt->name() );
                foreach( const QString& subgroup, fmtgrp.groupList() ) {
                    SourceFormatterStyle s( subgroup );
                    KConfigGroup stylegrp = fmtgrp.group( subgroup );
                    s.setCaption( stylegrp.readEntry( SourceFormatterController::styleCaptionKey, "" ) );
                    s.setContent( stylegrp.readEntry( SourceFormatterController::styleContentKey, "" ) );
                    sfmt.styles[ s.name() ] = s;
                }
            }
            l.formatters.insert( ifmt->name(), sfmt );
            languages[mime] = l;
        }
    }
    foreach( const QString& name, languages.keys() )
    {
        KConfigGroup grp = fmtctrl->configuration();
        QStringList formatter = grp.readEntry( name, "" ).split( "||", QString::KeepEmptyParts );
        SourceFormatterLanguage l = languages[name];
        if( formatter.count() != 2 || !l.formatters.contains( formatter.first() ) ) {
            l.selectedFmt = l.formatters.begin().key();
            SourceFormatter fmt = l.formatters[l.selectedFmt];
            if( !fmt.styles.isEmpty() )
            {
                fmt.selectedStyle = fmt.styles.begin().key();
            }
            l.formatters[ fmt.formatter->name() ] = fmt;
        } else {
            l.selectedFmt = formatter.first();
            SourceFormatter fmt = l.formatters[l.selectedFmt];
            fmt.selectedStyle = formatter.at( 1 );
            l.formatters[ fmt.formatter->name() ] = fmt;
        }
        languages[name] = l;
    }
    bool b = blockSignals( true );
    cbLanguages->blockSignals( !b );
    cbFormatters->blockSignals( !b );
    styleList->blockSignals( !b );
    chkKateModelines->blockSignals( !b );
    cbLanguages->clear();
    cbFormatters->clear();
    styleList->clear();
    chkKateModelines->setChecked( fmtctrl->configuration().readEntry( SourceFormatterController::kateModeLineConfigKey, false ) );
    foreach( const QString& name, languages.keys() )
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
}

void SourceFormatterSettings::save()
{
    KConfigGroup grp = Core::self()->sourceFormatterControllerInternal()->configuration();

    QSet<QString> savedFormatters;
    foreach( const QString& k, languages.keys() )
    {
        SourceFormatterLanguage l = languages[k];
        grp.writeEntry( l.mimeType, QString("%1||%2").arg(l.selectedFmt).arg(l.formatters[l.selectedFmt].selectedStyle ) );

        foreach( const QString& f, l.formatters.keys() )
        {
            if( !savedFormatters.contains( f ) )
            {
                SourceFormatter fmt = l.formatters[f];
                KConfigGroup fmtgrp = grp.group( fmt.formatter->name() );

                // delete all styles so we don't leave any behind when all user styles are deleted
                foreach( const QString& subgrp, fmtgrp.groupList() )
                {
                    if( subgrp.startsWith( userStylePrefix ) ) {
                        fmtgrp.deleteGroup( subgrp );
                    }
                }
                foreach( const QString& s, fmt.styles.keys() )
                {
                    SourceFormatterStyle style = fmt.styles[s];
                    if( style.name().startsWith( userStylePrefix ) )
                    {
                        KConfigGroup stylegrp = fmtgrp.group( style.name() );
                        stylegrp.writeEntry( SourceFormatterController::styleCaptionKey, style.caption() );
                        stylegrp.writeEntry( SourceFormatterController::styleContentKey, style.content() );
                    }
                }
            }
            savedFormatters << f;
        }
    }
    grp.writeEntry( SourceFormatterController::kateModeLineConfigKey, chkKateModelines->isChecked() );
    grp.sync();
}

void SourceFormatterSettings::enableStyleButtons()
{
    bool userEntry = styleList->currentItem()
                     && styleList->currentItem()->data( STYLE_ROLE ).toString().startsWith( userStylePrefix );

    SourceFormatterLanguage l = languages[cbLanguages->currentText()];
    ISourceFormatter* fmt = l.formatters[ l.selectedFmt ].formatter;
    bool hasEditWidget = ( fmt && fmt->editStyleWidget( KMimeType::mimeType( l.mimeType ) ) );
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
    SourceFormatterLanguage l = languages[cbLanguages->itemText( idx )];
    foreach( const QString& fmt, l.formatters.keys() )
    {
        cbFormatters->addItem( l.formatters[fmt].formatter->caption(), fmt );
    }
    cbFormatters->setCurrentIndex( cbFormatters->findData( l.selectedFmt ) );
    selectFormatter( cbFormatters->currentIndex() );
    emit changed( true );
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
    SourceFormatterLanguage l = languages[ cbLanguages->currentText() ];
    Q_ASSERT( idx < l.formatters.size() );
    Q_ASSERT( l.formatters.find( cbFormatters->itemData( idx ).toString() ) != l.formatters.end() );
    SourceFormatter fmt = l.formatters[ cbFormatters->itemData( idx ).toString() ];
    l.selectedFmt = fmt.formatter->name();
    languages[ cbLanguages->currentText() ] = l;
    foreach( const QString& style, fmt.styles.keys() )
    {
        QListWidgetItem* item = addStyle( fmt.styles[style] );
        if( style == fmt.selectedStyle )
        {
            styleList->setCurrentItem( item );
        }
    }
    enableStyleButtons();
    emit changed( true );
}

void SourceFormatterSettings::selectStyle( int row )
{
    if( row < 0 )
    {
        enableStyleButtons();
        return;
    }
    SourceFormatterLanguage l = languages[ cbLanguages->currentText() ];
    SourceFormatter fmt = l.formatters[ l.selectedFmt ];
    styleList->setCurrentRow( row );
    fmt.selectedStyle = styleList->item( row )->data( STYLE_ROLE ).toString();
    l.formatters[ l.selectedFmt ] = fmt;
    languages[ cbLanguages->currentText() ] = l;
    enableStyleButtons();
    updatePreview();
    emit changed( true );
}

void SourceFormatterSettings::deleteStyle()
{
    Q_ASSERT( styleList->currentRow() >= 0 );

    QListWidgetItem* item = styleList->takeItem( styleList->currentRow() );

    SourceFormatterLanguage l = languages[ cbLanguages->currentText() ];
    SourceFormatter fmt = l.formatters[ l.selectedFmt ];
    fmt.styles.remove( item->data( STYLE_ROLE ).toString() );
    l.formatters[ l.selectedFmt ] = fmt;
    languages[ cbLanguages->currentText() ] = l;
    delete item;
    selectStyle( styleList->count() > 0 ? 0 : -1 );
    updatePreview();
    emit changed( true );
}

void SourceFormatterSettings::editStyle()
{
    QString mimetype = cbLanguages->currentText();
    Q_ASSERT( languages.contains( mimetype ) );
    SourceFormatterLanguage l = languages[ mimetype ];
    SourceFormatter fmt = l.formatters[ l.selectedFmt ];

    if( fmt.formatter->editStyleWidget( KMimeType::mimeType( l.mimeType ) ) != 0 ) {
        EditStyleDialog dlg( fmt.formatter, KMimeType::mimeType( l.mimeType ), fmt.styles[ fmt.selectedStyle ], this );
        if( dlg.exec() == QDialog::Accepted )
        {
            SourceFormatterStyle s = fmt.styles[ fmt.selectedStyle ];
            s.setContent( dlg.content() );
            fmt.styles[ fmt.selectedStyle ] = s;
            l.formatters[ l.selectedFmt ] = fmt;
            languages[ mimetype ] = l;
        }
        updatePreview();
        emit changed( true );
    }
}

void SourceFormatterSettings::newStyle()
{
    QListWidgetItem* item = styleList->currentItem();
    SourceFormatterLanguage l = languages[ cbLanguages->currentText() ];
    SourceFormatter fmt = l.formatters[ l.selectedFmt ];
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
    SourceFormatterStyle s( QString( "%1%2" ).arg( userStylePrefix ).arg( idx ) );
    if( item ) {
        SourceFormatterStyle existstyle = fmt.styles[ item->data( STYLE_ROLE ).toString() ];
        s.setCaption( i18n( "New %1" ,existstyle.caption() ) );
        s.setContent( existstyle.content() );
    } else {
        s.setCaption( i18n( "New Style" ) );
    }
    fmt.styles[ s.name() ] = s;
    l.formatters[ l.selectedFmt ] = fmt;
    languages[ cbLanguages->currentText() ] = l;
    QListWidgetItem* newitem = addStyle( s );
    selectStyle( styleList->row( newitem ) );
    styleList->editItem( newitem );
    emit changed( true );
}

void SourceFormatterSettings::styleNameChanged( QListWidgetItem* item )
{
    if ( !item->isSelected() ) {
        return;
    }

    SourceFormatterLanguage l = languages[ cbLanguages->currentText() ];
    SourceFormatter fmt = l.formatters[ l.selectedFmt ];

    fmt.styles[ fmt.selectedStyle ].setCaption( item->text() );
    l.formatters[ l.selectedFmt ] = fmt;
    languages[ cbLanguages->currentText() ] = l;
    emit changed( true );
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
        SourceFormatterLanguage l = languages[ langName ];
        SourceFormatter fmt = l.formatters[ l.selectedFmt ];
        SourceFormatterStyle style = fmt.styles[ fmt.selectedStyle ];
        ISourceFormatter* ifmt = fmt.formatter;
        if( !ifmt ) {
            qWarning() << "oops, no formatter for:" << l.selectedFmt << l.mimeType;
        }
        KMimeType::Ptr mime = KMimeType::mimeType( l.mimeType );
        m_document->setHighlightingMode( ifmt->highlightModeForMime( mime ) );
        m_document->setText( ifmt->formatSourceWithStyle( style, ifmt->previewText( mime ), mime ) );
    } else
    {
        m_document->setText( i18n( "No Language selected" ) );
    }
    m_document->activeView()->setCursorPosition( KTextEditor::Cursor( 0, 0 ) );
    m_document->setReadWrite( false );
}

#include "sourceformattersettings.moc"
