#ifndef VALUE_H
#define VALUE_H
#include <QString>
#include "Serializable.h"


typedef int ValueTypes;
class Value : public Serializable
{
public:
    typedef enum {
        TypeValue = 0,
        TypeAMFBase = 0x50,
        TypeAMF3Base = 0x100
    } ValueTypesBase;


    virtual QString toString() const {
        return QString("[%1]").arg(valueToString());
    }

    virtual QString valueToString() const {
        return QString("value");
    }

    // Returns true if type is complex
    virtual bool isComplex() { return false; }

    // Make deep copy
    virtual Value * clone() const {
        return NULL;
    }

    bool operator<( const Value * other ) const {
        if (type() != other->type())
            return type() < other->type();
        return cmpInternal(other);
    }

    // Generic enum describing type (defined here ??)
    virtual ValueTypes type() const {
        return TypeValue;
    }
protected:
    // called when types are exact (that is value can be dynamic casted to other type)
    virtual bool cmpInternal(const Value * other) const = 0;
};

#endif // VALUE_H
