/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef OPTIONTABS_H
#define OPTIONTABS_H

#include <qwidget.h>

class QSpinBox;
class QRadioButton;
class FlagRadioButton;
class FlagCheckBoxController;
class FlagRadioButtonController;
class FlagPathEditController;
class FlagEditController;

class FeedbackTab : public QWidget
{
public:
    FeedbackTab( QWidget *parent=0, const char *name=0 );
    ~FeedbackTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
};

class FilesAndDirectoriesTab: public QWidget
{
public:
    FilesAndDirectoriesTab( QWidget *parent = 0, const char *name = 0);
    ~FilesAndDirectoriesTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagPathEditController *pathController;
};

class FilesAndDirectoriesTab2: public QWidget
{
public:
    FilesAndDirectoriesTab2( QWidget *parent = 0, const char *name = 0);
    ~FilesAndDirectoriesTab2();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagPathEditController *pathController;
};

class LanguageTab : public QWidget
{
public:
    LanguageTab( QWidget *parent=0, const char *name=0 );
    ~LanguageTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
};

class AssemblerTab : public QWidget
{
public:
    AssemblerTab( QWidget *parent=0, const char *name=0 );
    ~AssemblerTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagRadioButtonController *asmController;
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
    FlagRadioButtonController *optimController;

    QRadioButton *m_default;
    QRadioButton *m_default2;
    QRadioButton *m_default3;
    FlagRadioButton *optim1;
    FlagRadioButton *optim2;

private slots:
    void setReleaseOptions();
    void setDebugOptions();
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
};

class LinkerTab : public QWidget
{
public:
    LinkerTab( QWidget *parent=0, const char *name=0 );
    ~LinkerTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagEditController *listController;
};

class MiscTab : public QWidget
{
public:
    MiscTab( QWidget *parent=0, const char *name=0 );
    ~MiscTab();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

private:
    FlagCheckBoxController *controller;
    FlagRadioButtonController *radioController;
    FlagPathEditController *pathController;
    FlagEditController *editController;
};

#endif
