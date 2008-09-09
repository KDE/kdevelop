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
#ifndef SOURCEFORMATTERSETTINGS_H
#define SOURCEFORMATTERSETTINGS_H

#include <QHash>
#include <KCModule>
#include <KMimeType>

#include "ui_sourceformattersettings.h"

class QListWidgetItem;
namespace KTextEditor
{
	class Document;
	class View;
}
class ISourceFormatter;

/** Store : current plugins for each language
            current style for each language+formatter
*/
class SourceFormatterSettings : public KCModule, public Ui::SourceFormatterSettingsUI
{
		Q_OBJECT

	public:
		SourceFormatterSettings(QWidget *parent, const QVariantList &args);
		virtual ~SourceFormatterSettings();

	protected:
		void init();
		void poulateFormattersList();
		void populateStyleList();
		void setActiveLanguage(const QString &lang, const QString &plugin);

	public slots:
		virtual void load();
		virtual void save();
		void updatePreviewText();
		void addItemInStyleList(const QString &caption, const QString &name, bool editable = false);

	protected slots:
		void currentTabChanged();
		void languagesStylesChanged(int idx);
		void currentStyleChanged(QListWidgetItem *current, QListWidgetItem *previous);
		void styleRenamed(QListWidgetItem *item);
		void deleteStyle();
		void addStyle();
		void editStyle();
		void modelineChanged();

		void languagesFormattersChanged(int idx);
		void formattersChanged(int idx);

	private:
		KTextEditor::View *m_view;
		KTextEditor::Document *m_document;
		ISourceFormatter* m_currentFormatter;
		int m_numberOfPredefinedStyles;
		KMimeType::Ptr m_currentMimeType;
		QString m_previewText;
		QString m_currentLang;
};

#endif // SOURCEFORMATTERSETTINGS_H
