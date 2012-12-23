#ifndef VALUE_H
#define VALUE_H
#include <QString>
#include "Serializable.h"

class Value : public Serializable
{
public:
    virtual QString toString() const {
        return QString("[%1]").arg(valueToString());
    }

    virtual QString valueToString() const {
        return QString("value");
    }

    // Returns true if type is complex
    virtual bool isComplex() { return false; }
};

#endif // VALUE_H
