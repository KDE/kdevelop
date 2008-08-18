/* This file is part of KDevelop
*  Copyright (C) 2008 CÃ©dric Pasteur <cedric.pasteur@free.fr>
Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

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

#ifndef ASTYLEFORMATTER_H
#define ASTYLEFORMATTER_H

#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>

#include "astyle.h"
#include "astylewrapper_export.h"

class KSharedConfig;
template<class T> class KSharedPtr;

class ASTYLEWRAPPER_EXPORT AStyleFormatter : public astyle::ASFormatter
{
    public:
        /** Creates an empty AStyleFormatter with C style by default.
        */
        AStyleFormatter();
        AStyleFormatter(const QMap<QString, QVariant> &options);
        
        QString formatSource(const QString &text, const QString &indent=QString::null);
        
        QVariant option(const QString &name);
        void setOption(const QString &key, const QVariant &value);
        void setOptions(const QMap<QString, QVariant> &options);
        QString indentString();
        QString extensions() const;
        /**
        * Extensions from the widget passed in.
        * We preserve the order, so common extensions will
        * end up at the top
        * @param ext
        */
        void setExtensions(QString ext);
        bool hasExtension(const QString &extension);
        
        bool predefinedStyle(const QString &name);
        void loadConfig(const KSharedPtr<KSharedConfig> &config);
        void saveConfig(const KSharedPtr<KSharedConfig> &config);
        
        // indent
        void setTabIndentation(int length, bool forceTabs);
        void setSpaceIndentation(int length);
        void setTabSpaceConversionMode(bool mode);
        void setBlockIndent(bool on);
        void setBracketIndent(bool on);
        void setCaseIndent(bool on);
        void setClassIndent(bool on);
        void setLabelIndent(bool on);
        void setNamespaceIndent(bool on);
        void setPreprocessorIndent(bool on);
        void setSwitchIndent(bool on);
        void setMaxInStatementIndentLength(int max);
        void setMinConditionalIndentLength(int min);
        //brackets
        void setBracketFormatMode(astyle::BracketMode mode);
        void setBreakClosingHeaderBracketsMode(bool state);
        //blocks
        void setBreakBlocksMode(bool state);
        void setBreakElseIfsMode(bool state);
        void setBreakClosingHeaderBlocksMode(bool state);
        //padding
        void setOperatorPaddingMode(bool mode);
        void setParensOutsidePaddingMode(bool mode);
        void setParensInsidePaddingMode(bool mode);
        void setParensUnPaddingMode(bool state);
        //oneliners
        void setBreakOneLineBlocksMode(bool state);
        void setSingleStatementsMode(bool state);

    protected:
        void updateFormatter();
        void resetStyle();
        
    private:
        QString m_indentString;
        QMap<QString, QVariant> m_options;
        QStringList m_extensions;
        QMap<QString, QString> m_searchExtensions;
};

#endif // ASTYLEFORMATTER_H
