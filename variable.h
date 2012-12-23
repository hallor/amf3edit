#ifndef VARIABLE_H
#define VARIABLE_H
#include <QString>
#include "Serializable.h"

class Parser;
namespace amf3 {
class UTF_8_vr;
}
class QIODevice;

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
    const Serializable * value() const { return m_value; }
    Serializable * value() { return m_value; }

private:
    amf3::UTF_8_vr * m_name;
    Serializable * m_value;
};

#endif // VARIABLE_H
