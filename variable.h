#ifndef VARIABLE_H
#define VARIABLE_H
#include "Serializable.h"

struct Variable : public Serializable
{
    UTF_8_vr name;
    Serializable * value;

    variable() : value(NULL) {}

    virtual bool read(QIODevice & dev) {
        if (!name.read(dev))
            return false;
        if (value)
            delete value;
        value = parse_value(dev); // TODO: mem mgmt
        if (!value)
            return false;
        char x = 0;
        if (!dev.getChar(&x)) // One byte pad after key-value pair
            return false;
        if (x!=0) // This should not happen
            return false;
        return true;
    }

    virtual QString toString() const {
        return QString("<%1:%2>").arg(name.toString(), value->toString());
    }
};

#endif // VARIABLE_H
