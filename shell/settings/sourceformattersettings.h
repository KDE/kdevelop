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
#ifndef KDEVPLATFORM_SOURCEFORMATTERSETTINGS_H
#define KDEVPLATFORM_SOURCEFORMATTERSETTINGS_H

#include <QHash>
#include <QMimeType>

#include <interfaces/configpage.h>

#include "sourceformattercontroller.h"

#include "ui_sourceformattersettings.h"

class QListWidgetItem;

namespace KTextEditor
{
class Document;
class View;
}

namespace KDevelop
{
class ISourceFormatter;
class SourceFormatterStyle;
}

struct LanguageSettings {
    LanguageSettings();
    QList<QMimeType> mimetypes;
    QSet<KDevelop::SourceFormatter*> formatters;
    // weak pointers to selected formatter and style, no ownership
    KDevelop::SourceFormatter* selectedFormatter;     // Should never be zero
    KDevelop::SourceFormatterStyle* selectedStyle;  // TODO: can this be zero? Assume that not
};

/** \short The settings modulefor the Source formatter plugin.
* It supports predefined and custom styles. A live preview of the style
* is shown on the right side of the page.s
*/
class SourceFormatterSettings : public KDevelop::ConfigPage, public Ui::SourceFormatterSettingsUI
{
Q_OBJECT

public:
    explicit SourceFormatterSettings(QWidget* parent = 0);
    virtual ~SourceFormatterSettings();

    virtual QString name() const override;
    virtual QString fullName() const override;
    virtual QIcon icon() const override;

public slots:
    virtual void reset() override;
    virtual void apply() override;
    virtual void defaults() override;
private slots:
    void deleteStyle();
    void editStyle();
    void newStyle();
    void selectLanguage( int );
    void selectFormatter( int );
    void selectStyle( int );
    void styleNameChanged( QListWidgetItem* );
    void somethingChanged();
private:
    void updatePreview();
    QListWidgetItem* addStyle( const KDevelop::SourceFormatterStyle& s );
    static const QString userStylePrefix;
    void enableStyleButtons();
    // Language name -> language settings
    typedef QMap<QString, LanguageSettings> LanguageMap;
    LanguageMap languages;
    // formatter name -> formatter. Formatters owned by this
    typedef QMap<QString, KDevelop::SourceFormatter*> FormatterMap;
    FormatterMap formatters;
    KTextEditor::Document* m_document;
    KTextEditor::View* m_view;
};

#endif // KDEVPLATFORM_SOURCEFORMATTERSETTINGS_H
