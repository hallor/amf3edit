#ifndef VALUE_H
#define VALUE_H
#include <QString>
#include "Serializable.h"

class Value : public Serializable
{
public:
    virtual QString toString() const {
        return QString("[value]");
    }

    virtual QString valueAsString() const {
        return QString("??");
    }

    // Returns true if type is complex
    virtual bool isComplex() { return false; }
};

#endif // VALUE_H
