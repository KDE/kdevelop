/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEV_OUTPUTPAGE_H
#define KDEV_OUTPUTPAGE_H

#include <QWidget>
#include <KUrl>

#include "../languageexport.h"
#include <language/editor/simplecursor.h>

class KUrl;
namespace KDevelop {

class TemplateRenderer;
class SourceFileTemplate;
class CreateClassAssistant; 

class KDEVPLATFORMLANGUAGE_EXPORT OutputPage : public QWidget
{
    Q_OBJECT

public:
    OutputPage(QWidget* parent);
    virtual ~OutputPage();

    void loadFileTemplate(const KDevelop::SourceFileTemplate& fileTemplate, const KUrl& baseUrl, KDevelop::TemplateRenderer* renderer);

    QHash<QString, KUrl> fileUrls() const;
    QHash<QString, SimpleCursor> filePositions() const;

    bool isComplete() const;

Q_SIGNALS:
    void isValid(bool valid);

private:
    class OutputPagePrivate* const d;

private Q_SLOTS:
    virtual void updateFileNames();

    /**
     * This implementation simply enables the position widgets on a file that exists.
     * Derived classes should overload to set the ranges where class generation should be allowed
     *
     * @param field the name of the file to be generated (Header, Implementation, etc)
     */
    virtual void updateFileRange (const QString& field);
};

}

#endif // KDEV_OUTPUTPAGE_H
