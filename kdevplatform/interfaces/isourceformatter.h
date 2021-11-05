/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ISOURCEFORMATTER_H
#define KDEVPLATFORM_ISOURCEFORMATTER_H

#include <QWidget>

#include "interfacesexport.h"

class QUrl;
class QMimeType;
class QStringList;

namespace KDevelop
{

class KDEVPLATFORMINTERFACES_EXPORT SourceFormatterStyle
{
public:
	struct MimeHighlightPair {
		QString mimeType;
		QString highlightMode;
	};
	using MimeList = QVector<MimeHighlightPair>;

	SourceFormatterStyle();
	explicit SourceFormatterStyle( const QString& name );
	void setContent( const QString& content );
	void setCaption( const QString& caption );
	QString content() const;
	QString caption() const;
	QString name() const;
	QString description() const;
	void setDescription( const QString& desc );
	bool usePreview() const;
	void setUsePreview(bool use);

	void setMimeTypes( const MimeList& types );
	void setMimeTypes( const QStringList& types );

	/// Provides the possibility to the item to return a better-suited
	/// code sample. If empty, the default is used.
	QString overrideSample() const;
	void setOverrideSample( const QString& sample );

	MimeList mimeTypes() const;
	/// mime types as a QVariantList, type and mode separated by | in strings
	QVariant mimeTypesVariant() const;
	bool supportsLanguage(const QString& language) const;

	/// get the language / highlight mode for a given @p mime
	QString modeForMimetype(const QMimeType& mime) const;

	/// Copy content, mime types and code sample from @p other.
    void copyDataFrom(SourceFormatterStyle *other);

private:
	bool m_usePreview = false;
	QString m_name;
	QString m_caption;
	QString m_content;
	QString m_description;
	QString m_overrideSample;
	MimeList m_mimeTypes;
};

/**
 * @brief An object describing a style associated with a plugin
 *        which can deal with this style.
 */
struct SourceFormatterStyleItem {
	QString engine;
	SourceFormatterStyle style;
};

using SourceFormatterItemList = QVector<SourceFormatterStyleItem>;

/**
* @short A widget to edit a style
* A plugin should inherit this widget to create a widget to
* edit a style.
* @author Cédric Pasteur
*/
class KDEVPLATFORMINTERFACES_EXPORT SettingsWidget : public QWidget
{
		Q_OBJECT

	public:
		explicit SettingsWidget(QWidget *parent = nullptr);
		~SettingsWidget() override;

		/** This function is called after the creation of the dialog.
		* it should initialise the widgets with the values corresponding to
		* the predefined style \arg name if it's not empty, or
		* to the string \arg content.
		*/
		virtual void load(const SourceFormatterStyle&) = 0;

		/** \return A string representing the state of the config.
		*/
		virtual QString save() = 0;

	Q_SIGNALS:
		/** Emits this signal when a setting was changed and the preview
		* needs to be updated. \arg text is the text that will be shown in the
		* editor. One might want to show different text
		* according to the different pages shown in the widget.
		* Text should already be formatted.
		*/
		void previewTextChanged(const QString &text);
};

/**
 * @short An interface for a source beautifier
 * An example of a plugin using an external executable to do
 * the formatting can be found in kdevelop/plugins/formatters/indent_plugin.cpp.
 * @author Cédric Pasteur
 */
class KDEVPLATFORMINTERFACES_EXPORT ISourceFormatter
{
	public:
		virtual ~ISourceFormatter();

		/** \return The name of the plugin. This should contain only
		* ASCII chars and no spaces. This will be used internally to identify
		* the plugin.
		*/
		virtual QString name() const = 0;
		/** \return A caption describing the plugin.
		*/
		virtual QString caption() const = 0;
		/** \return A more complete description of the plugin.
		* The string should be written in Rich text. It can eg contain
		* a link to the project homepage.
		*/
		virtual QString description() const = 0;

		/**
		 * @return important information to display in style configuration UI
		 *
		 * @note The default implementation returns an empty string, which conveniently
		 *       saves UI space if there is no important information to display.
		 */
		virtual QString usageHint() const;

		/** Formats using the current style.
		 * @param text The text to format
		 * @param url The URL to which the text belongs (its contents must not be changed).
		 * @param leftContext The context at the left side of the text. If it is in another line, it must end with a newline.
		 * @param rightContext The context at the right side of the text. If it is in the next line, it must start with a newline.
		 *
		 * If the source-formatter cannot work correctly with the context, it will just return the input text.
		*/
		virtual QString formatSource(const QString &text, const QUrl& url, const QMimeType &mime, const QString& leftContext = QString(), const QString& rightContext = QString()) const = 0;

		/**
		 * Format with the given style, this is mostly for the kcm to format the preview text
		 * Its a bit of a hassle that this needs to be public API, but I can't find a better way
		 * to do this.
		 */
		virtual QString formatSourceWithStyle( SourceFormatterStyle,
											   const QString& text,
											   const QUrl& url,
											   const QMimeType &mime,
											   const QString& leftContext = QString(),
											   const QString& rightContext = QString() ) const = 0;

		/** \return A map of predefined styles (a key and a caption for each type)
		*/
		virtual QVector<SourceFormatterStyle> predefinedStyles() const = 0;

		/** \return The widget to edit a style.
		*/
		virtual SettingsWidget* editStyleWidget(const QMimeType &mime) const = 0;

		/** \return The text used in the config dialog to preview the current style.
		*/
		virtual QString previewText(const SourceFormatterStyle& style, const QMimeType &mime) const = 0;

		struct Indentation {
			Indentation() {
			}
			// If this indentation is really valid
			bool isValid() const {
				return indentationTabWidth != 0 || indentWidth != 0;
			}

			// The length of one tab used for indentation.
			// Zero if unknown, -1 if tabs should not be used for indentation
			int indentationTabWidth = 0;

			// The number of columns that equal one indentation level.
			// If this is zero, the default should be used.
			int indentWidth = 0;
		};

		/** \return The indentation of the style applicable for the given url.
		*/
		virtual Indentation indentation(const QUrl& url) const = 0;

		/** \return A string representing the map. Values are written in the form
		* key=value and separated with ','.
		*/
		static QString optionMapToString(const QMap<QString, QVariant> &map);
		/** \return A map corresponding to the string, that was created with
		* \ref optionMapToString.
		*/
		static QMap<QString, QVariant> stringToOptionMap(const QString &option);

		/** \return A message to display when an executable needed by a
		* plugin is missing. This should be returned as description
		* if a needed executable is not found.
		*/
		static QString missingExecutableMessage(const QString &name);
};

}

Q_DECLARE_INTERFACE(KDevelop::ISourceFormatter, "org.kdevelop.ISourceFormatter")
Q_DECLARE_TYPEINFO(KDevelop::SourceFormatterStyle::MimeHighlightPair, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::SourceFormatterStyle, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::SourceFormatterStyleItem, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_ISOURCEFORMATTER_H
