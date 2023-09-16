/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ISOURCEFORMATTER_H
#define KDEVPLATFORM_ISOURCEFORMATTER_H

#include <QMetaType>
#include <QWidget>

#include "interfacesexport.h"

#include <memory>

class QDebug;
class QUrl;
class QMimeType;
#include <QStringList>

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

	const QString& name() const
	{
		return m_name;
	}

	void setContent( const QString& content );
	void setCaption( const QString& caption );
	QString content() const;
	const QString& caption() const
	{
		return m_caption;
	}

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

	/// Copy everything but name, caption and description from @p other to *this.
	/// Call this function to set up a new user-defined style based on an existing style.
	void copyDataFrom(const SourceFormatterStyle& other);

private:
	bool m_usePreview = false;
	/// logically const: is modified only in compiler-generated special member functions
	QString m_name;
	QString m_caption;
	QString m_content;
	QString m_description;
	QString m_overrideSample;
	MimeList m_mimeTypes;
};

inline bool operator==(const SourceFormatterStyle::MimeHighlightPair& a,
                       const SourceFormatterStyle::MimeHighlightPair& b)
{
    return a.mimeType == b.mimeType && a.highlightMode == b.highlightMode;
}

KDEVPLATFORMINTERFACES_EXPORT QDebug operator<<(QDebug dbg, const SourceFormatterStyle& style);

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
		 *
		 * @param style a custom, not predefined, style to be edited
		 *
		 * @note This function must emit previewTextChanged().
		 */
		virtual void load(const SourceFormatterStyle& style) = 0;

		/** \return A string representing the state of the config.
		*/
		virtual QString save() const = 0;

	Q_SIGNALS:
		/**
		 * This signal is emitted whenever a setting changes and the preview needs to be updated.
		 *
		 * @param text non-empty code sample to be formatted and displayed as style preview
		 *
		 * @note The preview text may depend on the current page shown in the widget.
		 *       So @p text may be different each time this signal is emitted.
		 */
		void previewTextChanged(const QString &text);
};

using SettingsWidgetPtr = std::unique_ptr<SettingsWidget>;

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

		/**
		 * Format with the given style, this is mostly for the kcm to format the preview text
		 * Its a bit of a hassle that this needs to be public API, but I can't find a better way
		 * to do this.
		 */
		virtual QString formatSourceWithStyle( const SourceFormatterStyle& style,
											   const QString& text,
											   const QUrl& url,
											   const QMimeType &mime,
											   const QString& leftContext = QString(),
											   const QString& rightContext = QString() ) const = 0;

		/** \return A map of predefined styles (a key and a caption for each type)
		*/
		virtual QVector<SourceFormatterStyle> predefinedStyles() const = 0;

		/** \return A predefined style with the specified name or SourceFormatterStyle{name}
		* if this formatter has no such predefined style.
		*/
		SourceFormatterStyle predefinedStyle(const QString& name) const;

		/**
		 * @return whether editStyleWidget() returns a valid pointer
		 */
		virtual bool hasEditStyleWidget() const = 0;

		/** \return The widget to edit a style.
		*/
		virtual SettingsWidgetPtr editStyleWidget(const QMimeType &mime) const = 0;

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

		/** \return The indentation of @p style applicable for @p url and its MIME type @p mime
		*/
		virtual Indentation indentation(const SourceFormatterStyle& style, const QUrl& url,
		                                const QMimeType& mime) const = 0;

		/** \return A string representing the map. Values are written in the form
		* key=value and separated with ','.
		*/
		static QString optionMapToString(const QMap<QString, QVariant> &map);
		/** \return A map corresponding to the string, that was created with
		* \ref optionMapToString.
		*/
		static QMap<QString, QVariant> stringToOptionMap(const QString &option);

		/** \return The shared list of MIME types supported by each built-in style.
		*/
		static SourceFormatterStyle::MimeList mimeTypesSupportedByBuiltInStyles();

		/** \return A message to display when an executable needed by a
		* plugin is missing. This should be returned as description
		* if a needed executable is not found.
		*/
		static QString missingExecutableMessage(const QString &name);
};

}

Q_DECLARE_INTERFACE(KDevelop::ISourceFormatter, "org.kdevelop.ISourceFormatter")
Q_DECLARE_METATYPE(KDevelop::SourceFormatterStyle*)
Q_DECLARE_TYPEINFO(KDevelop::SourceFormatterStyle::MimeHighlightPair, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::SourceFormatterStyle, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::SourceFormatterStyleItem, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_ISOURCEFORMATTER_H
