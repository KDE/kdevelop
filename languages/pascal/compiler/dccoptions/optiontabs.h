/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef OPTIONTABS_H
#define OPTIONTABS_H

#include <qwidget.h>

class FlagRadioButtonController;
class FlagPathEditController;
class FlagCheckBoxController;
class FlagEditController;
class QRadioButton;
class FlagRadioButton;
class FlagCheckBox;

class LinkerTab : public QWidget
{
public:
    LinkerTab( QWidget *parent=0, const char *name=0 );
    ~LinkerTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagRadioButtonController *radioController;
    FlagPathEditController *pathController;
    FlagEditController *editController;
};

class LocationsTab : public QWidget
{
public:
    LocationsTab( QWidget *parent=0, const char *name=0 );
    ~LocationsTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagPathEditController *pathController;
};

class Locations2Tab : public QWidget
{
public:
    Locations2Tab( QWidget *parent=0, const char *name=0 );
    ~Locations2Tab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagPathEditController *pathController;
};

class GeneralTab: public QWidget
{
public:
    GeneralTab( QWidget *parent=0, const char *name=0 );
    ~GeneralTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagEditController *editController;
};

class CodegenTab : public QWidget
{
public:
    CodegenTab( QWidget *parent=0, const char *name=0 );
    ~CodegenTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagEditController *listController;
    FlagRadioButtonController *radioController;
};

class DebugOptimTab : public QWidget
{
    Q_OBJECT
public:
    DebugOptimTab( QWidget *parent=0, const char *name=0 );
    ~DebugOptimTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagRadioButtonController *radioController;

    FlagCheckBox *gdb;
    FlagCheckBox *namespacedb;
    FlagCheckBox *symboldb;

private slots:
    void setReleaseOptions();
    void setDebugOptions();
};

#endif
