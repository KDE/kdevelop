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
#include "editstyledialog.h"

#include <QVBoxLayout>
#include <KMessageBox>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/configinterface.h>
#include <KI18n/KLocalizedString>
#include <KUrl>

#include <interfaces/isourceformatter.h>

using KDevelop::ISourceFormatter;
using KDevelop::SettingsWidget;
using KDevelop::SourceFormatterStyle;

EditStyleDialog::EditStyleDialog(ISourceFormatter *formatter, const KMimeType::Ptr &mime,
        const SourceFormatterStyle &style, QWidget *parent)
		: KDialog(parent), m_sourceFormatter(formatter), m_mimeType(mime), m_style( style )
{
	m_content = new QWidget();
	m_ui.setupUi(m_content);
	setMainWidget(m_content);

	m_settingsWidget = m_sourceFormatter->editStyleWidget(mime);
	init();

	if (m_settingsWidget)
		m_settingsWidget->load(style);
}

EditStyleDialog::~EditStyleDialog()
{
}

void EditStyleDialog::init()
{
	// add plugin settings widget
	if(m_settingsWidget) {
		QVBoxLayout *layout = new QVBoxLayout(m_ui.settingsWidgetParent);
		layout->addWidget(m_settingsWidget);
		m_ui.settingsWidgetParent->setLayout(layout);
		connect(m_settingsWidget, SIGNAL(previewTextChanged(QString)),
			this, SLOT(updatePreviewText(QString)));
	}

	m_document = KTextEditor::Editor::instance()->createDocument(this);
	m_document->setReadWrite(false);
	QString mode = m_sourceFormatter->highlightModeForMime(m_mimeType);
	m_document->setHighlightingMode(mode);

	m_view = m_document->createView(m_ui.textEditor);
	QVBoxLayout *layout2 = new QVBoxLayout(m_ui.textEditor);
	layout2->addWidget(m_view);
	m_ui.textEditor->setLayout(layout2);
	m_view->show();

	KTextEditor::ConfigInterface *iface =
	    qobject_cast<KTextEditor::ConfigInterface*>(m_view);
	if (iface) {
		iface->setConfigValue("dynamic-word-wrap", false);
		iface->setConfigValue("icon-bar", false);
	}

	if (m_sourceFormatter)
		updatePreviewText(m_sourceFormatter->previewText(m_mimeType));
}

void EditStyleDialog::updatePreviewText(const QString &text)
{
	m_document->setReadWrite(true);
	m_style.setContent( content() );
	if (m_sourceFormatter) {
		m_document->setText(m_sourceFormatter->formatSourceWithStyle( m_style, text, KUrl(), m_mimeType ));
	} else {
		m_document->setText( i18n( "No Source Formatter available" ) );
	}

	m_view->setCursorPosition( KTextEditor::Cursor( 0, 0 ) );
	m_document->setReadWrite(false);
}

QString EditStyleDialog::content()
{
	if(m_settingsWidget)
		return m_settingsWidget->save();
	return QString();
}

#include "editstyledialog.moc"
// kate: indent-mode cstyle; space-indent off; tab-width 4;
