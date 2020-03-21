/* This file is part of KDevelop
   Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2011 David Nolden <david.nolden.kdevelop@art-master.de>

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
    explicit CustomScriptPlugin(QObject* parent, const QVariantList& = QVariantList());
    ~CustomScriptPlugin() override;

    QString name() const override;
    QString caption() const override;
    QString description() const override;

    /** Formats using the current style.
     */
    QString formatSource(const QString& text, const QUrl& url, const QMimeType& mime, const QString& leftContext, const QString& rightContext) const override;

    QString formatSourceWithStyle(KDevelop::SourceFormatterStyle, const QString& text,
                                  const QUrl& url,
                                  const QMimeType& mime,
                                  const QString& leftContext = QString(),
                                  const QString& rightContext = QString()) const override;

    /** \return A map of predefined styles (a key and a caption for each type)
     */
    QVector<KDevelop::SourceFormatterStyle> predefinedStyles() const override;

    /** \return The widget to edit a style.
     */
    KDevelop::SettingsWidget* editStyleWidget(const QMimeType& mime) const override;

    /** \return The text used in the config dialog to preview the current style.
     */
    QString previewText(const KDevelop::SourceFormatterStyle& style, const QMimeType& mime) const override;

    /** \return The indentation of the currently selected style.
     */
    Indentation indentation(const QUrl& url) const override;

private:
    QStringList computeIndentationFromSample(const QUrl& url) const;
    KDevelop::SourceFormatterStyle predefinedStyle(const QString& name) const;
};

class CustomScriptPreferences
    : public KDevelop::SettingsWidget
{
    Q_OBJECT
public:
    CustomScriptPreferences();

    void load (const KDevelop::SourceFormatterStyle& style) override;

    QString save() override;
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
