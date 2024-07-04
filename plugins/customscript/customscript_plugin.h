/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMSCRIPTPLUGIN_H
#define CUSTOMSCRIPTPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/isourceformatter.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class QTimer;

class CustomScriptPlugin
    : public KDevelop::IPlugin
    , public KDevelop::ISourceFormatter
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ISourceFormatter)
public:
    explicit CustomScriptPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~CustomScriptPlugin() override;

    QString name() const override;
    QString caption() const override;
    QString description() const override;
    QString usageHint() const override;

    QString formatSourceWithStyle(const KDevelop::SourceFormatterStyle& style,
                                  const QString& text,
                                  const QUrl& url,
                                  const QMimeType& mime,
                                  const QString& leftContext = QString(),
                                  const QString& rightContext = QString()) const override;

    /** \return A map of predefined styles (a key and a caption for each type)
     */
    QVector<KDevelop::SourceFormatterStyle> predefinedStyles() const override;

    bool hasEditStyleWidget() const override;

    /** \return The widget to edit a style.
     */
    KDevelop::SettingsWidgetPtr editStyleWidget(const QMimeType& mime) const override;

    /** \return The text used in the config dialog to preview the current style.
     */
    QString previewText(const KDevelop::SourceFormatterStyle& style, const QMimeType& mime) const override;

    Indentation indentation(const KDevelop::SourceFormatterStyle& style, const QUrl& url,
                            const QMimeType& mime) const override;

private:
    QStringList computeIndentationFromSample(const KDevelop::SourceFormatterStyle& style, const QUrl& url,
                                             const QMimeType& mime) const;
};

class CustomScriptPreferences
    : public KDevelop::SettingsWidget
{
    Q_OBJECT
public:
    CustomScriptPreferences();

    void load (const KDevelop::SourceFormatterStyle& style) override;

    QString save() const override;
private:
    QVBoxLayout* m_vLayout;
    QLabel* m_captionLabel;
    QHBoxLayout* m_hLayout;
    QLabel* m_commandLabel;
    QLineEdit* m_commandEdit;
    QLabel* m_bottomLabel;
    QTimer* m_updateTimer;
    QPushButton* m_moreVariablesButton;
    KDevelop::SourceFormatterStyle m_style;

    void updateTimeout();
    void moreVariablesClicked (bool);
};

#endif // CUSTOMSCRIPTPLUGIN_H
