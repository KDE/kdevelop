/*
 */
#ifndef ADA_UTILS_H
#define ADA_UTILS_H

#include <qstring.h>
#include "AdaAST.hpp"

QString qtext (const RefAdaAST& n);
QStringList qnamelist (const RefAdaAST& n);
QString ada_spec_filename (const QString& comp_unit_name);
QString fq_specfilename (const QString& comp_unit_name);

#endif

