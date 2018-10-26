/* This file is part of KDevelop
Copyright 2009 Andreas Pakulat <apaku@gmx.de>
Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
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
		/** \return Whether this mime type is supported by any plugin.
		*/
		virtual bool isMimeTypeSupported(const QMimeType &mime) = 0;

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
