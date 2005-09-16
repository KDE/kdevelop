
#include "problem.h"

Problem::Problem()
  : d(new Data())
{
}

Problem::Problem(const Problem &other)
  : d(other.d)
{
}

Problem &Problem::operator = (const Problem &other)
{
  d = other.d;
  return *this;
}

QString Problem::message() const
{
  return d->message;
}

void Problem::setMessage(const QString &message)
{
  d->message = message;
}

QString Problem::fileName() const
{
  return d->fileName;
}

void Problem::setFileName(const QString &fileName)
{
  d->fileName = fileName;
}

int Problem::line() const
{
  return d->line;
}

void Problem::setLine(int line)
{
  d->line = line;
}

int Problem::column() const
{
  return d->column;
}

void Problem::setColumn(int column)
{
  d->column = column;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
