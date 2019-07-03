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

#ifndef KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H
#define KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H

#include <interfaces/isourceformattercontroller.h>
#include <interfaces/isourceformatter.h>

#include <QSet>
#include <QVector>
#include <QMimeType>

#include <KXMLGUIClient>
#include <KConfigGroup>

#include "shellexport.h"

class QUrl;

namespace KTextEditor {
class Document;
}

namespace KDevelop
{

class Context;

class ContextMenuExtension;

class IDocument;
class IProject;
class TextDocument;
class ISourceFormatter;
class IPlugin;
class SourceFormatterControllerPrivate;

struct SourceFormatter
{
    KDevelop::ISourceFormatter* formatter;
    // style name -> style. style objects owned by this
    using StyleMap = QMap<QString,KDevelop::SourceFormatterStyle*>;
    StyleMap styles;
    // Get a list of supported mime types from the style map.
    QSet<QString> supportedMimeTypes() const
    {
        QSet<QString> supported;
        for ( auto style: styles ) {
            const auto mimeTypes = style->mimeTypes();
            for (auto& item : mimeTypes) {
                supported.insert(item.mimeType);
            }
        }
        return supported;
    }
    SourceFormatter() = default;
    ~SourceFormatter()
    {
        qDeleteAll(styles);
    };
private:
    Q_DISABLE_COPY(SourceFormatter)
};

/** \short A singleton class managing all source formatter plugins
*/
class KDEVPLATFORMSHELL_EXPORT SourceFormatterController : public ISourceFormatterController, public KXMLGUIClient
{
    Q_OBJECT

    friend class SourceFormatterJob;

public:
    static QString kateModeLineConfigKey();
    static QString kateOverrideIndentationConfigKey();
    static QString styleCaptionKey();
    static QString styleContentKey();
    static QString styleMimeTypesKey();
    static QString styleSampleKey();

    explicit SourceFormatterController(QObject *parent = nullptr);
    ~SourceFormatterController() override;
    void initialize();
    void cleanup();
    //----------------- Public API defined in interfaces -------------------
    /** \return The formatter corresponding to the language
    * of the document corresponding to the \arg url.
    */
    ISourceFormatter* formatterForUrl(const QUrl &url) override;
    /** Loads and returns a source formatter for this mime type.
    * The language is then activated and the style is loaded.
    * The source formatter is then ready to use on a file.
    */
    ISourceFormatter* formatterForUrl(const QUrl& url, const QMimeType& mime) override;
    bool hasFormatters() const override;
    /** \return Whether this mime type is supported by any plugin.
    */
    bool isMimeTypeSupported(const QMimeType& mime) override;

    /**
    * @brief Instantiate a Formatter for the given plugin and load its configuration.
    *
    * @param ifmt The ISourceFormatter interface of the plugin
    * @return KDevelop::SourceFormatter* the SourceFormatter instance for the plugin, including config items
    */
    SourceFormatter* createFormatterForPlugin(KDevelop::ISourceFormatter* ifmt) const;

    /**
    * @brief Find the first formatter which supports a given mime type.
    */
    ISourceFormatter* findFirstFormatterForMimeType(const QMimeType& mime) const;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent);

    KDevelop::SourceFormatterStyle styleForUrl(const QUrl& url, const QMimeType& mime) override;

    KConfigGroup configForUrl(const QUrl& url) const;
    KConfigGroup sessionConfig() const;
    KConfigGroup globalConfig() const;

    void settingsChanged();

    void disableSourceFormatting(bool disable) override;
    bool sourceFormattingEnabled() override;

    QVector<KDevelop::ISourceFormatter*> formatters() const;

Q_SIGNALS:
    void formatterLoaded(KDevelop::ISourceFormatter* ifmt);
    void formatterUnloading(KDevelop::ISourceFormatter* ifmt);

private Q_SLOTS:
    void updateFormatTextAction();
    void beautifySource();
    void beautifyLine();
    void formatFiles();
    void documentLoaded(const QPointer<KDevelop::TextDocument>& doc);
    void projectOpened(const KDevelop::IProject* project);
    void pluginLoaded(KDevelop::IPlugin* plugin);
    void unloadingPlugin(KDevelop::IPlugin* plugin);

private:
    /** \return A modeline string (to add at the end or the beginning of a file)
    * corresponding to the settings of the active language.
    */
    QString addModelineForCurrentLang(QString input, const QUrl& url, const QMimeType&);
    /** \return The name of kate indentation mode for the mime type.
    * examples are cstyle, python, etc.
    */
    QString indentationMode(const QMimeType& mime);
    void formatDocument(KDevelop::IDocument* doc, ISourceFormatter* formatter, const QMimeType& mime);
    // Adapts the mode of the editor regarding indentation-style
    void adaptEditorIndentationMode(KTextEditor::Document* doc, KDevelop::ISourceFormatter* formatter,
                                    const QUrl& url, bool ignoreModeline = false);

    void resetUi();

private:
    const QScopedPointer<class SourceFormatterControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(SourceFormatterController)
};

}

#endif // KDEVPLATFORM_SOURCEFORMATTERMANAGER_H
