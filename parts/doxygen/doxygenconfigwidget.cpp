/***************************************************************************
 *   Copyright (C) 2000 by Dimitri van Heesch                              *
 *   dimitri@stack.nl                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
***************************************************************************/

#include "doxygenconfigwidget.h"

#include <qscrollview.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "config.h"
#include "input.h"
#include "messages.h"


DoxygenConfigWidget::DoxygenConfigWidget(const QString &fileName, QWidget *parent, const char *name)
    : QTabWidget(parent, name)
{
    m_hasChanged = false;
    m_dependencies = new QDict< QPtrList<IInput> >(257);
    m_dependencies->setAutoDelete(true);
    m_inputWidgets = new QDict< IInput >;
    m_switches = new QDict< QObject >;
   
    QPtrListIterator<ConfigOption> options = Config::instance()->iterator();
    QScrollView *page = 0;
    QVBox *pagebox = 0;
    ConfigOption *option = 0;
    for (options.toFirst(); (option=options.current()); ++options) {
        switch(option->kind())
            {
            case ConfigOption::O_Info:
                page = new QScrollView(this, option->name());
                page->viewport()->setBackgroundMode(PaletteBackground);
                pagebox = new QVBox(0);
                Q_ASSERT(pagebox!=0);
                page->addChild(pagebox);
                addTab(page, message(option->name()));
                QWhatsThis::add(page, option->docs().simplifyWhiteSpace() );
                break;
            case ConfigOption::O_String:
                {
                    Q_ASSERT(pagebox!=0);
                    InputString::StringMode sm = InputString::StringFree;
                    switch (((ConfigString *)option)->widgetType()) {
                    case ConfigString::String: sm = InputString::StringFree; break;
                    case ConfigString::File:   sm = InputString::StringFile; break;
                    case ConfigString::Dir:    sm = InputString::StringDir;  break;
                    }
                    InputString *inputString = new InputString
                        ( message(option->name()),               // name
                          pagebox,                               // widget
                          *((ConfigString *)option)->valueRef(), // variable 
                          sm                                     // type
                          );
                    QWhatsThis::add(inputString, option->docs().simplifyWhiteSpace());
                    connect(inputString, SIGNAL(changed()), this, SLOT(changed()));
                    m_inputWidgets->insert(option->name(), inputString);
                    addDependency(m_switches, option->dependsOn(), option->name());
                }
                break;
            case ConfigOption::O_Enum:
                {
                    Q_ASSERT(pagebox!=0);
                    InputString *inputString = new InputString
                        ( message(option->name()),               // name
                          pagebox,                               // widget
                          *((ConfigEnum *)option)->valueRef(),   // variable 
                          InputString::StringFixed               // type
                          );
                    QStrListIterator sli=((ConfigEnum *)option)->iterator();
                    for (sli.toFirst(); sli.current(); ++sli)
                        inputString->addValue(sli.current());
                    QWhatsThis::add(inputString, option->docs().simplifyWhiteSpace());
                    connect(inputString, SIGNAL(changed()), this, SLOT(changed()));
                    m_inputWidgets->insert(option->name(),inputString);
                    addDependency(m_switches, option->dependsOn(), option->name());
                }
                break;
            case ConfigOption::O_List:
                {
                    Q_ASSERT(pagebox!=0);
                    InputStrList::ListMode lm = InputStrList::ListString;
                    switch(((ConfigList *)option)->widgetType())
                        {
                        case ConfigList::String:     lm=InputStrList::ListString;  break;
                        case ConfigList::File:       lm=InputStrList::ListFile;    break;
                        case ConfigList::Dir:        lm=InputStrList::ListDir;     break;
                        case ConfigList::FileAndDir: lm=InputStrList::ListFileDir; break;
                        }
                    InputStrList *inputStrList = new InputStrList
                        ( message(option->name()),                // name
                          pagebox,                                // widget
                          *((ConfigList *)option)->valueRef(),    // variable
                          lm                                      // type
                          );
                    QWhatsThis::add(inputStrList, option->docs().simplifyWhiteSpace());
                    connect(inputStrList, SIGNAL(changed()), this, SLOT(changed()));
                    m_inputWidgets->insert(option->name(),inputStrList);
                    addDependency(m_switches, option->dependsOn(), option->name());
                }
                break;
            case ConfigOption::O_Bool:
                {
                    Q_ASSERT(pagebox!=0);
                    InputBool *inputBool = new InputBool
                        ( option->name(),                         // key
                          message(option->name()),                // name
                          pagebox,                                // widget
                          *((ConfigBool *)option)->valueRef()     // variable
                          );
                    QWhatsThis::add(inputBool, option->docs().simplifyWhiteSpace());
                    connect(inputBool, SIGNAL(changed()), this, SLOT(changed()));
                    m_inputWidgets->insert(option->name(), inputBool);
                    addDependency(m_switches, option->dependsOn(), option->name());
                }
                break;
            case ConfigOption::O_Int:
                {
                    Q_ASSERT(pagebox!=0);
                    InputInt *inputInt = new InputInt
                        ( message(option->name()),                // name
                          pagebox,                                // widget
                          *((ConfigInt *)option)->valueRef(),     // variable
                          ((ConfigInt *)option)->minVal(),        // min value
                          ((ConfigInt *)option)->maxVal()         // max value
                          );
                    QWhatsThis::add(inputInt, option->docs().simplifyWhiteSpace());
                    connect(inputInt, SIGNAL(changed()), this, SLOT(changed()));
                    m_inputWidgets->insert(option->name(), inputInt);
                    addDependency(m_switches, option->dependsOn(), option->name());
                }
                break;
            case ConfigOption::O_Obsolete:
                break;
            }
    }
    
    QDictIterator<QObject> di(*m_switches);
    for (; di.current(); ++di) {
        QObject *obj = di.current();
        connect(obj, SIGNAL(toggle(const QString&, bool)), this, SLOT(toggle(const QString&, bool)));
        // UGLY HACK: assumes each item depends on a boolean without checking!
        emit toggle(di.currentKey(), ((InputBool *)obj)->getState());
    }
   
    m_fileName = fileName;
    loadFile();
}


DoxygenConfigWidget::~DoxygenConfigWidget()
{
    delete m_dependencies;
    delete m_inputWidgets;
    delete m_switches;
}


QSize DoxygenConfigWidget::sizeHint() const
{
    // without this the whole dialog becomes much too large
    return QSize(QTabWidget::sizeHint().width(), 1);
}


void DoxygenConfigWidget::addDependency(QDict<QObject> *switches,
                                        const QCString &dep, const QCString &name)
{
    if (dep.isEmpty())
        return;
    
    IInput *parent = m_inputWidgets->find(dep);
    Q_ASSERT(parent!=0);
    IInput *child = m_inputWidgets->find(name);
    Q_ASSERT(child!=0);
    if (!switches->find(dep))
        switches->insert(dep, parent->qobject());
    QPtrList<IInput> *list = m_dependencies->find(dep);
    if (!list) {
        list = new QPtrList<IInput>;
        m_dependencies->insert(dep, list);
    }
    list->append(child);
}


void DoxygenConfigWidget::toggle(const QString &name, bool state)
{
    QPtrList<IInput> *inputs = m_dependencies->find(name);
    Q_ASSERT(inputs!=0);
    IInput *input = inputs->first();
    while (input) {
        input->setEnabled(state);
        input = inputs->next();
    }
}


void DoxygenConfigWidget::changed()
{
    m_hasChanged = true;
}


void DoxygenConfigWidget::init()
{
    QDictIterator<IInput> di(*m_inputWidgets);
    for (; di.current(); ++di)
        di.current()->init();
    
    QDictIterator<QObject> dio(*m_switches);
    for (; dio.current(); ++dio) {
        QObject *obj = dio.current();
        connect(obj, SIGNAL(toggle(const QString&, bool)), this, SLOT(toggle(const QString&, bool)));
        // UGLY HACK: assumes each item depends on a boolean without checking!
        emit toggle(dio.currentKey(), ((InputBool *)obj)->getState());
    }
}


void DoxygenConfigWidget::loadFile()
{
    Config::instance()->init();

    QFile f(m_fileName);
    if (f.open(IO_ReadOnly)) {
        QTextStream is(&f);

        Config::instance()->parse(QFile::encodeName(m_fileName));
        Config::instance()->convertStrToVal();

        f.close();
    }

    init();
}


void DoxygenConfigWidget::saveFile()
{
    QFile f(m_fileName);
    if (!f.open(IO_WriteOnly)) {
        KMessageBox::information(0, i18n("Cannot write Doxyfile."));
    } else {
        QTextStream t(&f);

        Config::instance()->writeTemplate(t, true, false);

        f.close();
    }
}


void DoxygenConfigWidget::accept()
{
    if (m_hasChanged)
        saveFile();
}

#include "doxygenconfigwidget.moc"
