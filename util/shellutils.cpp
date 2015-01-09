/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Ivan Shapovalov <intelfx100@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "shellutils.h"
#include <QGuiApplication>
#include <QtCore/QTextStream>
#include <KLocalizedString>
#include <KMessageBox>

namespace KDevelop {

bool askUser( const QString& mainText,
              const QString& ttyPrompt,
              const QString& mboxTitle,
              const QString& mboxAdditionalText,
              bool ttyDefaultToYes )
{
    if( !qobject_cast<QGuiApplication*>(qApp) ) {
        // no ui-mode e.g. for duchainify and other tools
        QTextStream out( stdout );
        out << mainText << endl;
        QTextStream in( stdin );
        QString input;
        forever {
            if( ttyDefaultToYes ) {
                out << QStringLiteral( "%1: [Y/n] " ).arg( ttyPrompt ) << flush;
            } else {
                out << QStringLiteral( "%1: [y/N] ").arg( ttyPrompt ) << flush;
            }
            input = in.readLine().trimmed();
            if( input.isEmpty() ) {
                return ttyDefaultToYes;
            } else if( input.toLower() == "y" ) {
                return true;
            } else if( input.toLower() == "n" ) {
                return false;
            }
        }
    } else {
        int userAnswer = KMessageBox::questionYesNo( 0,
                                                     mainText + "\n\n" + mboxAdditionalText,
                                                     mboxTitle,
                                                     KStandardGuiItem::ok(),
                                                     KStandardGuiItem::cancel() );
        return userAnswer == KMessageBox::Yes;
    }
}


}
