/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ISOURCEFORMATTERCONTROLLER_H
#define KDEVPLATFORM_ISOURCEFORMATTERCONTROLLER_H

#include <QObject>

#include "interfacesexport.h"

class QUrl;
class QMimeType;

namespace KDevelop
{
class ISourceFormatter;
class SourceFormatterStyle;

/** \short An interface to the controller managing all source formatter plugins
 */
class KDEVPLATFORMINTERFACES_EXPORT ISourceFormatterController : public QObject
{
		Q_OBJECT

	public:
		explicit ISourceFormatterController(QObject *parent = nullptr);
		~ISourceFormatterController() override;

		/** \return The formatter corresponding to the language
		* of the document corresponding to the \p url.
		* The language is then activated and the style is loaded.
		* The source formatter is then ready to use.
		* If mimetype of url is known already, use
		* formatterForUrl(const QUrl& url, const QMimeType& mime) instead.
		*/
		virtual ISourceFormatter* formatterForUrl(const QUrl &url) = 0;
		/** \return The formatter corresponding to the language
		* of the document corresponding to the \p url.
		* The language is then activated and the style is loaded.
		* The source formatter is then ready to use.
		* @param mime known mimetype of the url
		*/
		virtual ISourceFormatter* formatterForUrl(const QUrl& url, const QMimeType& mime) = 0;
		///\return @c true if there are formatters at all, @c false otherwise
		virtual bool hasFormatters() const = 0;

		virtual KDevelop::SourceFormatterStyle styleForUrl(const QUrl& url, const QMimeType& mime) = 0;

		///Set whether or not source formatting is disabled with \arg disable
		virtual void disableSourceFormatting(bool disable) = 0;
		///\return Whether or not source formatting is enabled
		virtual bool sourceFormattingEnabled() = 0;

	Q_SIGNALS:
		void hasFormattersChanged(bool hasFormatters);
};

}

#endif // KDEVPLATFORM_ISOURCEFORMATTERCONTROLLER_H
