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

#include "editstyledialog.h"

#define STYLE_ROLE (Qt::UserRole+1)

K_PLUGIN_FACTORY(SourceFormatterSettingsFactory, registerPlugin<SourceFormatterSettings>();)
K_EXPORT_PLUGIN(SourceFormatterSettingsFactory("kcm_kdevsourceformattersettings"))

using KDevelop::Core;
using KDevelop::ISourceFormatter;
using KDevelop::SourceFormatterController;
using KDevelop::SourceFormatterLanguage;
using KDevelop::SourceFormatterStyle;
using KDevelop::SourceFormatterCfg;

SourceFormatterSettings::SourceFormatterSettings(QWidget *parent, const QVariantList &args)
		: KCModule(SourceFormatterSettingsFactory::componentData(), parent, args), m_view(0), m_document(0)
{
	setupUi(this);

	// add texteditor preview
	KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
	if (!editor) {
		KMessageBox::error(this, i18n("A KDE text-editor component could not be found.\n"
		        "Please check your KDE installation."));
        return;
    }

	m_document = editor->createDocument(this);

	m_view = qobject_cast<KTextEditor::View*>(m_document->createView(textEditor));
	QVBoxLayout *layout = new QVBoxLayout(textEditor);
	layout->addWidget(m_view);
	textEditor->setLayout(layout);
	m_view->show();

	KTextEditor::ConfigInterface *iface =
	    qobject_cast<KTextEditor::ConfigInterface*>(m_view);
	if (iface) {
		iface->setConfigValue("dynamic-word-wrap", false);
		iface->setConfigValue("icon-bar", false);
	}

	// set buttons icons
	btnNewStyle->setIcon(KIcon("list-add"));
	btnDelStyle->setIcon(KIcon("list-remove"));
	btnEditStyle->setIcon(KIcon("configure"));

	connect(cbLanguagesStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(languagesStylesChanged(int)));
	connect(listStyles, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
	        this, SLOT(currentStyleChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(listStyles, SIGNAL(itemChanged(QListWidgetItem*)),
	        this, SLOT(styleRenamed(QListWidgetItem*)));
	connect(btnDelStyle, SIGNAL(clicked()), this, SLOT(deleteStyle()));
	connect(btnNewStyle, SIGNAL(clicked()), this, SLOT(addStyle()));
	connect(btnEditStyle, SIGNAL(clicked()), this, SLOT(editStyle()));
	connect(chkKateModelines, SIGNAL(stateChanged(int)), this, SLOT(modelineChanged()));
	connect(cbFormatters, SIGNAL(currentIndexChanged(int)), this, SLOT(formattersChanged(int)));
}

SourceFormatterSettings::~SourceFormatterSettings()
{
	delete m_document;
}

void SourceFormatterSettings::addItemInStyleList(const SourceFormatterStyle &style, bool editable)
{
	QListWidgetItem *item = new QListWidgetItem(style.caption());
	item->setData(STYLE_ROLE, style.name());
	if (editable)
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	listStyles->addItem(item);
}

void SourceFormatterSettings::updatePreviewText()
{
	m_document->setReadWrite(true);

	// Fallback if the following code doesn't set a proper text
	m_document->setText("No Formatter Available");
	SourceFormatterController* ctrl = Core::self()->sourceFormatterControllerInternal();
	SourceFormatterLanguage lang = currentLanguage();
	SourceFormatterCfg cfg = lang.formatters[lang.selectedFormatter];
	ISourceFormatter* fmt = cfg.fmt;
	if( fmt )
	{
		KMimeType::Ptr mime = KMimeType::mimeType( lang.mimeType );
		QString previewText = fmt->previewText( mime );
		m_document->setHighlightingMode(fmt->highlightModeForMime(mime));
		m_document->setText(ctrl->formatSourceForLanguage(previewText, lang));
	}
	m_document->setReadWrite(false);
}

void SourceFormatterSettings::load()
{
	bool blockSig = blockSignals( true );
	cbLanguagesStyle->clear();
	listStyles->clear();
	cbFormatters->clear();
	Core::self()->sourceFormatterControllerInternal()->loadConfig();
	//init language combo box
	SourceFormatterController *manager = Core::self()->sourceFormatterControllerInternal();
	foreach(const SourceFormatterLanguage &l, manager->languages())
	{
		m_languages.append(l);
		KIcon icon(manager->iconForLanguage(l));
		cbLanguagesStyle->addItem(icon, l.mimeType, QVariant::fromValue( l ));
	}

	if( cbLanguagesStyle->count() > 0 )
	{
		cbLanguagesStyle->setCurrentIndex(0);
	} else
	{
		// Invalid index, so the rest is disabled
		cbLanguagesStyle->setCurrentIndex(-1);
	}
	checkEnabled();
	//update kate modeline
	chkKateModelines->setChecked(Core::self()->sourceFormatterControllerInternal()->modelinesEnabled());

	blockSignals( blockSig );
}

void SourceFormatterSettings::checkEnabled()
{
	cbLanguagesStyle->setEnabled( ( cbLanguagesStyle->count() > 0 ) );
	cbFormatters->setEnabled( cbFormatters->count() > 0 && cbLanguagesStyle->currentIndex() >= 0 );
	listStyles->setEnabled( cbFormatters->count() > 0  && cbFormatters->currentIndex() >= 0 );
	btnNewStyle->setEnabled( cbFormatters->count() > 0 && cbFormatters->currentIndex() >= 0 );
	btnDelStyle->setEnabled( listStyles->currentItem() && (listStyles->currentItem()->flags() & Qt::ItemIsEditable) != 0);
	btnEditStyle->setEnabled( listStyles->currentItem() && (listStyles->currentItem()->flags() & Qt::ItemIsEditable) != 0);
}

void SourceFormatterSettings::save()
{
	//TODO:: Need to think about how to store "changes", we need to keep track of the formatter+currentstyle combo for each language
	for( int i = 0; i < cbLanguagesStyle->count(); i++ )
	{
		QVariant data = cbLanguagesStyle->itemData( i );
		if( data.isValid() && data.canConvert<KDevelop::SourceFormatterLanguage>() )
		{
			Core::self()->sourceFormatterControllerInternal()->updateFormatterLanguage( data.value<KDevelop::SourceFormatterLanguage>() );
		}
	}
	Core::self()->sourceFormatterControllerInternal()->setModelinesEnabled(chkKateModelines->isChecked());
	Core::self()->sourceFormatterControllerInternal()->saveConfig();
}

void SourceFormatterSettings::languagesStylesChanged(int idx)
{
	cbFormatters->clear();
	if (idx < 0) // no selection
	{
		// Make sure the formatter has an invalid index if we have an invalid one
		cbFormatters->setCurrentIndex( -1 );
		return;
	}

	QVariant data = cbLanguagesStyle->itemData( idx );
	if( data.isValid() && data.canConvert<KDevelop::SourceFormatterLanguage>() )
	{
		SourceFormatterLanguage lang = data.value<KDevelop::SourceFormatterLanguage>();
		foreach( SourceFormatterCfg cfg, lang.formatters )
		{
			cbFormatters->addItem( cfg.fmt->caption(), cfg.fmt->name() );
		}
		int idx = -1;
		if( lang.selectedFormatter != "" )
		{
			idx = cbFormatters->findData( lang.selectedFormatter  );
		}
		cbFormatters->setCurrentIndex( idx );
	}
	
	checkEnabled();
	changed();
}

void SourceFormatterSettings::populateStyleList( ISourceFormatter* fmt )
{
	//add predefined styles
	QList<SourceFormatterStyle> styles;
	if(fmt)
	{
		styles = fmt->predefinedStyles();
	}

        foreach( const SourceFormatterStyle& style, styles )
        {
                addItemInStyleList( style );
        }

        styles = Core::self()->sourceFormatterControllerInternal()->userStyles( fmt->name() );

        foreach( const SourceFormatterStyle& style, styles )
        {
                addItemInStyleList( style, true );
        }
}

void SourceFormatterSettings::currentStyleChanged(QListWidgetItem *current, QListWidgetItem *)
{
	SourceFormatterLanguage lang = currentLanguage();
	if( current )
	{
		lang.formatters[lang.selectedFormatter].selectedStyle = current->data( STYLE_ROLE ).toString();
	} else
	{
		lang.formatters[lang.selectedFormatter].selectedStyle = "";
	}
	cbLanguagesStyle->setItemData( cbLanguagesStyle->currentIndex(), QVariant::fromValue<KDevelop::SourceFormatterLanguage>( lang ) );
	checkEnabled();
	changed();
	updatePreviewText();
}

void SourceFormatterSettings::styleRenamed(QListWidgetItem * current)
{
	SourceFormatterLanguage lang = currentLanguage();
        SourceFormatterCfg cfg = lang.formatters[lang.selectedFormatter];
        SourceFormatterStyle style = cfg.styles[cfg.selectedStyle];
        style.setCaption( current->data( STYLE_ROLE ).toString() );
        cfg.styles[cfg.selectedStyle] = style;
        lang.formatters[lang.selectedFormatter] = cfg;
	changed();
}

void SourceFormatterSettings::deleteStyle()
{

	int res = KMessageBox::questionYesNo(this, i18n("Are you sure you"
	        " want to delete this style?"), i18n("Delete style"));
	if (res == KMessageBox::No)
	{
		return;
	}

	//remove list item
	int idx = listStyles->currentRow();
	QListWidgetItem *item = listStyles->takeItem(idx);
	if (!item)
	{
		return;
	}
	QString styleName = item->data(STYLE_ROLE).toString();

	for( int i = 0; i < cbLanguagesStyle->count(); i++ ) 
	{
		QVariant data = cbLanguagesStyle->itemData( i );
		Q_ASSERT( data.isValid() && data.canConvert<SourceFormatterLanguage>() );
		SourceFormatterLanguage lang = data.value<KDevelop::SourceFormatterLanguage>();
		if( lang.formatters[lang.selectedFormatter].selectedStyle == item->data( STYLE_ROLE ).toString() ) 
		{
			QString newStyle;
			if( listStyles->count() > 0 )
			{
				newStyle = listStyles->item( 0 )->data( STYLE_ROLE ).toString();
			} else {
				newStyle = "";
			}
			lang.formatters[lang.selectedFormatter].selectedStyle = newStyle;
			cbLanguagesStyle->setItemData( i, QVariant::fromValue( lang ) );
		}
	}
	Core::self()->sourceFormatterControllerInternal()->deleteStyle(currentLanguage().selectedFormatter, styleName);
	delete item;
	if( listStyles->count() > 0 )
	{
		listStyles->setCurrentRow(idx - 1);
	}
	checkEnabled();
	changed();
}

void SourceFormatterSettings::addStyle()
{
	//ask for caption
	bool ok;
	QString caption = QInputDialog::getText(this,
	        i18n("New style"), i18n("Please enter a name for the new style"),
	        QLineEdit::Normal, i18n("Custom Style"), &ok);
	if (!ok) // dialog aborted
	{
		return;
	}

	SourceFormatterStyle style;
	SourceFormatterLanguage lang = currentLanguage();
	SourceFormatterCfg cfg = lang.formatters[lang.selectedFormatter];
	ISourceFormatter* fmt = cfg.fmt;
	KMimeType::Ptr mime = KMimeType::mimeType( lang.mimeType );
	// This shouldn't be possible as the new-button should be disabled if there is no selected formatter
	Q_ASSERT(fmt);
	QListWidgetItem *item = listStyles->currentItem();
	
	// if user has selected a style, use it as base
	if(item)
	{
			style = cfg.styles[cfg.selectedStyle];
	} else // just use first predefined style as base
	{
			style = fmt->predefinedStyles().at(0);
	}

	EditStyleDialog dialog(fmt, mime, style);
	if (dialog.exec() == QDialog::Accepted) {
		SourceFormatterController *manager = Core::self()->sourceFormatterControllerInternal();
		SourceFormatterStyle newstyle = manager->newStyle(lang.selectedFormatter);
		newstyle.setContent( dialog.content() );
		newstyle.setCaption( caption );
		manager->saveStyle(lang.selectedFormatter, newstyle);
		// add item in list and select it
		addItemInStyleList(newstyle, true);
		foreach( QListWidgetItem* item, listStyles->findItems( caption, Qt::MatchExactly ) )
		{
			if( item->data( STYLE_ROLE ).toString() == newstyle.name() )
			{
				listStyles->setCurrentItem(item);
				break;
			}
		}
	}

	changed();
}

void SourceFormatterSettings::editStyle()
{
	QListWidgetItem *item = listStyles->currentItem();
	if (!item)
	{
		return;
	}
	QString styleName = item->data(STYLE_ROLE).toString();

	SourceFormatterController *manager = Core::self()->sourceFormatterControllerInternal();
	
	KDevelop::SourceFormatterLanguage lang = currentLanguage();
        SourceFormatterCfg cfg = lang.formatters[lang.selectedFormatter];
        SourceFormatterStyle style = cfg.styles[cfg.selectedStyle];

	// Should never get here, edit button is supposed to be disabled if there's no formatter selected
	Q_ASSERT(cfg.fmt);

	EditStyleDialog dialog(cfg.fmt, KMimeType::mimeType( lang.mimeType ), style);
	if (dialog.exec() == QDialog::Accepted)
		Core::self()->sourceFormatterControllerInternal()->saveStyle(lang.selectedFormatter, style);
}

void SourceFormatterSettings::modelineChanged()
{
	changed();
}

void SourceFormatterSettings::formattersChanged(int idx)
{
	listStyles->clear();
	// invalid index, lets clear the stylelist
	if( idx < 0 )
	{
		checkEnabled();
		return;
	}
	SourceFormatterLanguage lang = currentLanguage();
	KMimeType::Ptr mime = KMimeType::mimeType( lang.mimeType );
	ISourceFormatter* fmt = lang.formatters[ cbFormatters->itemData( idx ).toString() ].fmt;
	Q_ASSERT(fmt);
	lang.selectedFormatter = fmt->name();
	populateStyleList( fmt );

	QString style = lang.formatters[ fmt->name() ].selectedStyle;

	// Fallback to first predefined if there exist one
	if( style.isEmpty() && !fmt->predefinedStyles().isEmpty() )
	{
		style = fmt->predefinedStyles().at(0).name();
	}

	if( !style.isEmpty() )
	{
		lang.formatters[ lang.selectedFormatter ].selectedStyle = style;
		for (int i = 0; i < listStyles->count(); ++i)
		{
			QListWidgetItem *item = listStyles->item(i);
			if (item->data(STYLE_ROLE).toString() == style)
			{
				listStyles->setCurrentRow(i);
				break;
			}
		}
	}
	updateCurrentLanguage( lang );
	changed();
}

void SourceFormatterSettings::updateCurrentLanguage( KDevelop::SourceFormatterLanguage lang )
{
    cbLanguagesStyle->setItemData( cbLanguagesStyle->currentIndex(), QVariant::fromValue( lang ) );
}

KDevelop::SourceFormatterLanguage SourceFormatterSettings::currentLanguage()
{
	QVariant data = cbLanguagesStyle->itemData( cbLanguagesStyle->currentIndex() );
	Q_ASSERT( data.isValid() && data.canConvert<KDevelop::SourceFormatterLanguage>() );
	return data.value<KDevelop::SourceFormatterLanguage>();
}

#include "sourceformattersettings.moc"
// kate: indent-mode cstyle; space-indent off; tab-width 4;
