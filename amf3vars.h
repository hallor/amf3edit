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
    void write(QIODevice & dev) ;

    int value() const { return m_value; }
    void setValue(int v) { m_value = v; }

    QString toString() const {
        return QString("[U29:%1]").arg(m_value);
    }

};

class UTF_8_vr : public Value
{
    int ref;
    QString value;

public:
    UTF_8_vr();

    bool operator<(const UTF_8_vr & other) const;

    void read(QIODevice & dev) ;

    QString toString() const {
        if (ref >=0)
            return QString("[utf-8-r:%1]").arg(ref);
        else
            return QString("[utf-8-v:'%1']").arg(value);
    }

};

class undefined_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) {
        dev.putChar(0); // Just marker
    }

    QString toString() const {
        return QString("[undefined]");
    }

};

class null_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) {
        dev.putChar(0x1); // Just marker
    }

    QString toString() const {
        return QString("[null]");
    }

};

class false_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) {
        dev.putChar(0x2); // Just marker
    }

    QString toString() const {
        return QString("[false]");
    }

};

class true_type : public Value
{
public:
    void read(QIODevice & /*dev*/) {    }

    void write(QIODevice & dev) {
        dev.putChar(0x3); // Just marker
    }

    QString toString() const {
        return QString("[true]");
    }

};

class integer_type : public U29
{
public:
    void write(QIODevice &dev) const ;

    QString toString() const {
        return QString("[int:%1]").arg(U29::toString());
    }
};

class string_type : public UTF_8_vr
{
public:
    QString toString() const {
        return QString("[string:%1]").arg(UTF_8_vr::toString());
    }
};

class array_type : public Value // TODO: sparse arrays
{
    QVector<Value*> data;
    QMap<UTF_8_vr, Value*> assoc; // assoc part
public:

    void read(QIODevice & dev, const Parser & parser) ;

    bool isComplex() const { return true; }

    QString toString() const;
};

} // namespace
#endif // AMF3VARS_H
