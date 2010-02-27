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
#ifndef SOURCEFORMATTERSETTINGS_H
#define SOURCEFORMATTERSETTINGS_H

#include <QHash>
#include <KCModule>
#include <KMimeType>

#include "interfaces/isourceformatter.h"

#include "ui_sourceformattersettings.h"

class QListWidgetItem;

namespace KTextEditor
{
class Document;
}

namespace KDevelop
{
class ISourceFormatter;
}

struct SourceFormatter
{
    KDevelop::ISourceFormatter* formatter;
    QMap<QString,KDevelop::SourceFormatterStyle> styles;
    QString selectedStyle;
};

struct SourceFormatterLanguage
{
    QString mimeType;
    QMap<QString,SourceFormatter> formatters;
    QString selectedFmt;
};

/** \short The settings modulefor the Source formatter plugin.
* It supports predefined and custom styles. A live preview of the style
* is shown on the right side of the page.s
*/
class SourceFormatterSettings : public KCModule, public Ui::SourceFormatterSettingsUI
{
Q_OBJECT

public:
    SourceFormatterSettings( QWidget *parent, const QVariantList &args );
    virtual ~SourceFormatterSettings();

public slots:
    virtual void load();
    virtual void save();
private slots:
    void deleteStyle();
    void editStyle();
    void newStyle();
    void selectLanguage( int );
    void selectFormatter( int );
    void selectStyle( int );
    void styleNameChanged( QListWidgetItem* );
private:
    void updatePreview();
    QListWidgetItem* addStyle( const KDevelop::SourceFormatterStyle& s );
    static const QString userStylePrefix;
    void enableStyleButtons();
    QMap<QString,SourceFormatterLanguage> languages;
    KTextEditor::Document* m_document;
};

#endif // SOURCEFORMATTERSETTINGS_H
