/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "cppxmlparse.h"
#include <KDebug>

int XmlToKross::start()
{
    inclass=0;
    while (!xml.atEnd())
    {
        QXmlStreamReader::TokenType t=xml.readNext(); 
        QString str;
        
        switch(t)
        {
            case QXmlStreamReader::Invalid:
                qDebug() << "invalid token!" << xml.errorString() << endl;
                break;
            case QXmlStreamReader::StartDocument:
                writeDocument();
                break;
            case QXmlStreamReader::StartElement:
//                         qDebug() << "Element " << xml.name().toString() << endl;
                str=xml.name().toString();
                if(str=="Class")
                {
                    inclass++;
                    if(inclass==1) writeClass(xml.attributes().value(QString(), "name").toString(), QString(), QList<QStringList>());
                }
                else if(str=="Function")
                {
                    QString funcname=xml.attributes().value(QString(), "name").toString();
                    QString rettype=xml.attributes().value(QString(), "type_name").toString();
                    
                    bool isConst=xml.attributes().value(QString(), "constant").toString()==QChar('1');
                    bool isVirtual=xml.attributes().value(QString(), "virtual").toString()==QChar('1');
                    currentMethod=method();
                    currentMethod.access=xml.attributes().value(QString(), "access").toString();
                    currentMethod.funcname=funcname;
                    currentMethod.returnType=rettype;
                    currentMethod.isConst=isConst;
                    currentMethod.isVirtual=isVirtual;
                    if(!definedClasses.isEmpty() && ( funcname==definedClasses.last() || funcname=='~'+definedClasses.last() ) )
                        currentMethod.access="constructor";
                }
                else if(str=="Argument")
                {
                    method::argument arg;
                    arg.name=xml.attributes().value(QString(), "name").toString();
                    arg.type=xml.attributes().value(QString(), "type_name").toString();
                    arg.def=xml.attributes().value(QString(), "defaultvalue").toString();
                    QString context=xml.attributes().value(QString(), "context").toString();
                    if(!arg.def.isEmpty() && arg.type.startsWith(context))
                        arg.def.prepend(context+"::");
                    
                    currentMethod.args.append(arg);
                }
                else if(str=="Namespace")
                {
                    QString name=xml.attributes().value(QString(), "name").toString();
                    
                    qDebug() << "entering to namespace " << name << inNamespace;
                    if(inNamespace!=name)
                    {
                        if(!inNamespace.isEmpty())
                            inNamespace.append("::");
                        inNamespace.append(name);
                        writeNamespace(name);
                        qDebug() << "current status" << inNamespace;
                    }
                }
                else if(str=="Variable" && inclass==1 &&
                    xml.attributes().value(QString(), "access").toString()=="public")
                {
                    writeVariable(xml.attributes().value(QString(), "name").toString(),
                                    xml.attributes().value(QString(), "type_name").toString(), xml.attributes().value(QString(), "type_constant").toString()=="1");
                }
                else if(str=="Enum")
                {
                    flags.clear();
                    flags.append(xml.attributes().value(QString(), "name").toString());
                }
                else if(str=="Enumerator")
                    flags.append(xml.attributes().value(QString(), "name").toString());
                break;
            case QXmlStreamReader::EndDocument:
                writeEndDocument();
                break;
            case QXmlStreamReader::EndElement:
                str=xml.name().toString();
                if(str=="Class")
                {
                    if(inclass==1) writeEndClass();
                    inclass--;
                }
                else if(str=="Function" && currentMethod.access=="public" && inclass==1)
                    writeEndFunction(currentMethod);
                else if(str=="Enum")
                    writeEndEnum(flags);
                else if(str=="Namespace")
                {
                    inNamespace.resize(inNamespace.lastIndexOf("::"));
                }
                break;
            default:
                break;
        }
    }
    if (xml.error())
    {
        qDebug() << "error!" << xml.errorString();
        return 2;
    }
    return 0;
}
