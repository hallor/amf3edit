#ifndef VARIABLE_H
#define VARIABLE_H
#include <QString>
#include "Value.h"

class Parser;
namespace amf3 {
class UTF_8_vr;
}
class QIODevice;
class Variable : public Value
{
public:
    Variable(const Parser & parser);
    ~Variable();

    void read(QIODevice & dev);

    QString toString() const;

    bool isComplex();

    QString name() const;
    const Value* value() const { return m_value; }
    Value * value() { return m_value; }

    ValueTypes type() const {
        return TypeVariable;
    }
protected:
    bool cmpInternal(const Value * other) const;
private:
    const Parser & m_parser;
    amf3::UTF_8_vr * m_name;
    Value * m_value;
};

#endif // VARIABLE_H
