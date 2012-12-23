#ifndef VARIABLE_H
#define VARIABLE_H
#include <QString>
#include "Serializable.h"

class Parser;
namespace amf3 {
class UTF_8_vr;
}
class QIODevice;
class Value;
class Variable : public Serializable
{
public:
    Variable();
    ~Variable();

    void read(QIODevice & dev, const Parser & parser);

    QString toString() const;

    bool isComplex()
    {
        if (m_value)
            return m_value->isComplex();
        return false;
    }

    QString name() const;
    const Value* value() const { return m_value; }
    Value * value() { return m_value; }

private:
    amf3::UTF_8_vr * m_name;
    Value * m_value;
};

#endif // VARIABLE_H
