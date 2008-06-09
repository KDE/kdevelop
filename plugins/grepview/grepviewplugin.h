/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEWPART_H_
#define _GREPVIEWPART_H_

#include <iplugin.h>
#include <QtCore/QVariant>

class GrepDialog;

namespace KDevelop
{
    class IProject;
}

class GrepOutputDelegate;

class GrepViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    GrepViewPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~GrepViewPlugin();

    GrepOutputDelegate* delegate() const;

private Q_SLOTS:
    void slotGrep();
    void showDialogWithPattern(const QString& pattern);
    void searchActivated();

private:
    QString currentWord();
    QString currentSelectedWord();

    GrepDialog* m_grepdlg;

    KDevelop::IProject *m_projectForActiveFile;
    GrepOutputDelegate* m_delegate;
};

#endif
