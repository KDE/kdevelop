/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASTYLEPLUGIN_H
#define ASTYLEPLUGIN_H

#include "astyle_preferences.h"

#include <interfaces/iplugin.h>
#include <interfaces/isourceformatter.h>

class AStyleFormatter;

class AStylePlugin : public KDevelop::IPlugin, public KDevelop::ISourceFormatter
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ISourceFormatter)

public:
    explicit AStylePlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~AStylePlugin() override;

    QString name() const override;
    QString caption() const override;
    QString description() const override;

    /** \return A map of predefined styles (a key and a caption for each type)
    */
    QVector<KDevelop::SourceFormatterStyle> predefinedStyles() const override;

    bool hasEditStyleWidget() const override;

    /** \return The widget to edit a style.
    */
    KDevelop::SettingsWidgetPtr editStyleWidget(const QMimeType& mime) const override;

    QString formatSourceWithStyle(const KDevelop::SourceFormatterStyle& style,
                                  const QString& text,
                                  const QUrl &url,
                                  const QMimeType& mime,
                                  const QString& leftContext = QString(),
                                  const QString& rightContext = QString()) const override;

    /** \return The text used in the config dialog to preview the current style.
    */
    QString previewText(const KDevelop::SourceFormatterStyle& style, const QMimeType& mime) const override;

    Indentation indentation(const KDevelop::SourceFormatterStyle& style, const QUrl& url,
                            const QMimeType& mime) const override;

    static QString formattingSample(AStylePreferences::Language lang);
    static QString indentingSample(AStylePreferences::Language lang);

private:
    QScopedPointer<AStyleFormatter> m_formatter;
};

#endif // ASTYLEPLUGIN_H
