#ifndef AMF3VARS_H
#define AMF3VARS_H
#include <QString>
#include <QIODevice>
#include <QMap>
#include <QVector>
#include "Value.h"

class Parser;
namespace amf3 {

typedef enum {
    Amf3Base = Value::TypeAMF3Base,
    TypeU29,
    TypeUtf8,
    TypeArray,
    TypeFalse,
    TypeTrue,
    TypeInteger,
    TypeString,
    TypeUndefined,
    TypeNull
} AMF3Types;

class U29 : public Value {
    int m_value;
public:

    U29();
    U29(const U29 * other);

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

    Value * clone() const
    {
        return new U29(this);
    }

    bool cmpInternal( const Value * other ) const
    {
        return m_value < dynamic_cast<const U29*>(other)->m_value;
    }

    ValueTypes type() const {
        return TypeU29;
    }
};

class UTF_8_vr : public Value
{
    int ref;
    QString m_value;
public:
    UTF_8_vr() : ref(-1), m_value("undefined") {}
    UTF_8_vr(const UTF_8_vr * other) : ref(other->ref), m_value(other->m_value) {}

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

    Value * clone() const
    {
        UTF_8_vr *v = new UTF_8_vr(this);
        return v;
    }

    bool cmpInternal( const Value * other ) const
    {
        const UTF_8_vr * v = dynamic_cast<const UTF_8_vr*>(other);
        if (ref != v->ref)
            return ref < v->ref;
        return m_value < v->m_value;
    }

    ValueTypes type() const {
        return TypeUtf8;
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

    Value * clone() const
    {
       return new undefined_type();
    }

    bool cmpInternal( const Value * /*other*/ ) const
    { // All are equal
        return false;
    }

    ValueTypes type() const {
        return TypeUndefined;
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

    Value * clone() const
    {
       return new null_type();
    }

    bool cmpInternal( const Value * /*other*/ ) const
    { // All are equal
        return false;
    }

    ValueTypes type() const {
        return TypeNull;
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

    Value * clone() const
    {
       return new false_type();
    }

    bool cmpInternal( const Value * /*other*/ ) const
    { // All are equal
        return false;
    }

    ValueTypes type() const {
        return TypeFalse;
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

    Value * clone() const
    {
       return new true_type();
    }

    bool cmpInternal( const Value * /*other*/ ) const
    { // All are equal
        return false;
    }

    ValueTypes type() const {
        return TypeTrue;
    }

};

class integer_type : public U29
{
public:
    integer_type() : U29() {}
    integer_type(const integer_type * other) : U29(other) {}

    void write(QIODevice &dev) const;

    QString toString() const {
        return QString("[int:%1]").arg(value());
    }

    QString valueToString() const {
        return QString::number(value());
    }

    Value * clone() const
    {
       return new integer_type(this);
    }

    ValueTypes type() const {
        return TypeInteger;
    }
};

class string_type : public UTF_8_vr
{
public:
    string_type() : UTF_8_vr() {}
    string_type(const string_type * other) : UTF_8_vr(other) {}

    QString toString() const {
        return QString("[string:%1]").arg(UTF_8_vr::value());
    }

    Value * clone() const
    {
       return new string_type(this);
    }

    ValueTypes type() const {
        return TypeString;
    }
};

class array_type : public Value // TODO: sparse arrays
{
    QVector<Value*> m_data;
    QMap<UTF_8_vr, Value*> m_assoc; // assoc part
    const Parser & m_parser;
public:
    array_type(const Parser & p) : m_parser(p) {}

    array_type(const array_type * other);

    void read(QIODevice & dev);

    bool isComplex() const { return true; }

    QString toString() const;

    const QVector<Value *> & value() const { return m_data; }

    Value * clone() const
    {
       return new array_type(this);
    }

    bool cmpInternal( const Value * other ) const;

    ValueTypes type() const {
        return TypeArray;
    }
};

} // namespace
#endif // AMF3VARS_H
