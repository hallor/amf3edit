#ifndef SOLREADER_H
#define SOLREADER_H
#include <QString>
#include "variable.h"

class SolFile
{
public:
    SolFile();

    bool load(QIODevice & from);
    bool save(QIODevice & to) const;

    QString rootName() const;
    int version() const;
    bool bigEndian() const;

    // Editable tree, variables have one instance and should be not removed for now
    QList<Variable*> getTree();
private:
    QString rootName;
    int version;
    bool bigEndian;
    QList<Variable*> data;
    bool valid;
};

#endif // SOLREADER_H
