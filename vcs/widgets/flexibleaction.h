/***************************************************************************
 *   Copyright 2011 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_COPYACTION_H
#define KDEVPLATFORM_COPYACTION_H

#include <QAction>

namespace KDevelop
{

class AbstractFunction {
    public:
        virtual ~AbstractFunction();
        virtual void operator()() = 0;
};

class FlexibleAction : public QAction
{
    Q_OBJECT
    public:
        explicit FlexibleAction(const QIcon& icon, const QString& text, AbstractFunction* function, QObject* parent);
            
    public slots:
        void actionTriggered(bool);
    
    private:
        QScopedPointer<AbstractFunction> m_function;
};

}
#endif
