#ifndef PROBLEM_H
#define PROBLEM_H

#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

class Problem
{
private:
  Problem();
  Problem(const Problem &other);
  Problem &operator = (const Problem &other);

  QString message() const;
  void setMessage(const QString &message);

  QString fileName() const;
  void setFileName(const QString &fileName);

  int line() const;
  void setLine(int line);

  int column() const;
  void setColumn(int column);

private:
  struct Data: public QSharedData
  {
    QString message;
    QString fileName;
    int line;
    int column;

    Data():
      line(0), column(0) {}
  };
  QSharedDataPointer<Data> d;
};

#endif // PROBLEM_H

// kate: space-indent on; indent-width 2; replace-tabs on;
