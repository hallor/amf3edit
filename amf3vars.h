#ifndef AMF3VARS_H
#define AMF3VARS_H
#include <QString>
#include <QIODevice>
#include <QMap>
#include <QVector>
#include "Value.h"

class Parser;
namespace amf3 {

class U29 : public Value {
    int m_value;
public:

    U29();

    void read(QIODevice & dev) ;
    void write(QIODevice & dev) const;

    int value() const { return m_value; }
    void setValue(int v) { m_value = v; }

    QString toString() const {
        return QString("[U29:%1]").arg(m_value);
    }

    QString valueToString() const {
        return QString("%1").arg(m_value);
    }
};

class UTF_8_vr : public Value
{
    int ref;
    QString m_value;
public:
    bool operator<(const UTF_8_vr & other) const;

    void read(QIODevice & dev) ;

    QString toString() const {
        if (ref >=0)
            return QString("[utf-8-r:%1]").arg(valueToString());
        else
            return QString("[utf-8-v:%1]").arg(valueToString());
    }

    QString valueToString() const {
        if (ref >=0)
            return QString("@%1").arg(ref);
        else
            return QString("'%1'").arg(m_value);
    }

    QString value() const {
        if (ref >=0)
            return QString("@%1").arg(ref);
        else
            return QString("%1").arg(m_value);
    }

    void setValue(const QString & v)
    {
        m_value = v;
    }

};

class undefined_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) const {
        dev.putChar(0); // Just marker
    }

    QString valueToString() const {
        return QString("undefined");
    }

};

class null_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) const{
        dev.putChar(0x1); // Just marker
    }

    QString valueToString() const {
        return QString("null");
    }

};

class false_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) const {
        dev.putChar(0x2); // Just marker
    }

    QString valueToString() const {
        return QString("false");
    }

};

class true_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) const {
        dev.putChar(0x3); // Just marker
    }

    QString valueToString() const {
        return QString("true");
    }

};

class integer_type : public U29
{
public:
    void write(QIODevice &dev) const;

    QString toString() const {
        return QString("[int:%1]").arg(value());
    }

    QString valueToString() const {
        return QString::number(value());
    }
};

class string_type : public UTF_8_vr
{
public:
    QString toString() const {
        return QString("[string:%1]").arg(UTF_8_vr::value());
    }
};

class array_type : public Value // TODO: sparse arrays
{
    QVector<Value*> data;
    QMap<UTF_8_vr, Value*> assoc; // assoc part
    const Parser & parser;
public:
    array_type(const Parser & p) : parser(p) {}

    void read(QIODevice & dev);

    bool isComplex() const { return true; }

    QString toString() const;
};

} // namespace
#endif // AMF3VARS_H
