/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H
#define KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H

#include <interfaces/isourceformattercontroller.h>
#include <interfaces/isourceformatter.h>

#include <QUrl>
#include <QVector>
#include <QMimeType>

#include <KXMLGUIClient>
#include <KConfigGroup>

#include "shellexport.h"

#include <functional>
#include <map>

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
    static QString styleShowPreviewKey();
    static QString styleContentKey();
    static QString styleMimeTypesKey();
    static QString styleSampleKey();

    explicit SourceFormatterController(QObject *parent = nullptr);
    ~SourceFormatterController() override;
    void initialize();
    void cleanup();
    //----------------- Public API defined in interfaces -------------------
    FileFormatterPtr fileFormatter(const QUrl& url) const override;
    bool hasFormatters() const override;

    /// style name -> style; heterogeneous lookup is enabled
    using StyleMap = std::map<QString, SourceFormatterStyle, std::less<>>;

    /**
     * @return all styles that belong to @p formatter
     */
    StyleMap stylesForFormatter(const KDevelop::ISourceFormatter& formatter) const;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent);

    KConfigGroup sessionConfig() const;
    KConfigGroup globalConfig() const;

    void settingsChanged();

    void disableSourceFormatting() override;
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
    void pluginLoaded(KDevelop::IPlugin* plugin);
    void unloadingPlugin(KDevelop::IPlugin* plugin);

private:
    class FileFormatter final : public IFileFormatter
    {
    public:
        /**
         * @param doc the document to be formatted
         */
        explicit FileFormatter(const IDocument& doc);

        /**
         * @param url URL of the file to be formatted
         *
         * @note Prefer the more efficient IDocument constructor overload.
         */
        explicit FileFormatter(QUrl url);

        /**
         * Read and store in this object user-configured formatter and style for our file.
         * @param formatters loaded and available formatters
         * @return @e true on success, otherwise @e false
         * @note This function is the second and last initialization step after the constructor.
         *       Call it once. Do not call any other non-static member function (other than the
         *       destructor) before this function is called and succeeds.
         */
        bool readFormatterAndStyle(const QVector<ISourceFormatter*>& formatters);

        QString formatterCaption() const;
        QString styleCaption() const;

        QString format(const QString& text, const QString& leftContext = QString(),
                       const QString& rightContext = QString()) const override;

        /**
         * @return @p input with a modeline string corresponding to source formatter configuration for our file
         * @note A modeline within @p input is adjusted or a new one is appended to @p input.
         */
        QString addModeline(QString input) const;

        /**
         * Format the open document.
         * @param doc our file's document
         */
        void formatDocument(IDocument& doc) const;

        /**
         * Adapt the mode of the editor regarding indentation style.
         */
        void adaptEditorIndentationMode(KTextEditor::Document* doc, bool ignoreModeline = false) const;

        /**
         * Adapt global formatting state to a newly opened project @p project.
         */
        static void projectOpened(const IProject& project, const QVector<ISourceFormatter*>& formatters);

    private:
        explicit FileFormatter(QUrl&& url, QMimeType&& mimeType, const KConfigGroup& sourceFormatterConfig,
                               const ISourceFormatter* formatter, SourceFormatterStyle&& style);

        QUrl m_url;
        const QMimeType m_mimeType; ///< the MIME type of @a m_url
        KConfigGroup m_sourceFormatterConfig; ///< is determined by @a m_url
        /**
         * The names of @a m_formatter and @a m_style are read from the entry of @a m_sourceFormatterConfig
         * at key=@a m_mimeType.name(). @a m_formatter and @a m_style themselves are then formed based on
         * @e SourceFormatterController's loaded formatters and on global style configuration.
         */
        const ISourceFormatter* m_formatter = nullptr;
        SourceFormatterStyle m_style;
    };

    void resetUi();

private:
    const QScopedPointer<class SourceFormatterControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(SourceFormatterController)
};

}

#endif // KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H
