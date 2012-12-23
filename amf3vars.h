#ifndef AMF3VARS_H
#define AMF3VARS_H
#include "Serializable.h"

namespace amf3 {

struct U29 : public Serializable {

    int m_value;

    U29() : m_value(42) {}

    bool read(QIODevice & dev)
    {
        m_value=0;

        int i;
        for (i=0; i<4; i++) { // up num 4 bytes
            quint8 c;
            if (!dev.read((char*)&c, 1))
                return false;

            if (i != 3) {
                m_value = m_value << 7;
                m_value |= c & ~0x80;
            }
            else { // Last char has 8 bits
                m_value = m_value << 8;
                m_value |= c;
            }

            if ( (c & 0x80) == 0 )  // End of number
                break;
        }
        if (m_value > 0x3fffFFFF)
            return false;
        return true;
    }

    virtual bool write(QIODevice & dev) { // TODO: refactor
        bool ret = true;
        ret &= dev.putChar(0x4); // Marker
#if 0
        int bytes;
        if (value <= 0x7f) { // 1 byte
            bytes = 1;
        } else if (value <= 0x3fff) { // 2 bytes
            bytes = 2;
        } else if (value <= 0x1fFFFF) { // 3 bytes
            bytes = 3;
        } else if (value <= 0x3fffFFFF) { // 4 bytes
            bytes = 4;
        } else // out of range
            return false;
#endif
        return ret;
    }


    QString toString() const {
        return QString("[U29:%1]").arg(m_value);
    }

};

struct UTF_8_vr : public Serializable
{
    int ref;
    QString value;

    bool operator<(const UTF_8_vr & other) const {
        if (ref >=0 )
            return ref < other.ref;
        else
            return value < other.value;
    }

    bool read(QIODevice & dev)
    {
        U29 m;
        if (!m.read(dev))
            return false;
        if (m.m_value & 0x1) { // value
            ref = -1;
            value = dev.read(m.m_value >> 1);
            if (dev.atEnd() && value.length() == 0) // detect EOF
                return false;
        } else { // ref
            ref = m.m_value >> 1;
        }
        return true;
    }

    QString toString() const {
        if (ref >=0)
            return QString("[utf-8-r:%1]").arg(ref);
        else
            return QString("[utf-8-v:'%1']").arg(value);
    }

};

struct undefined_type : public Serializable
{
    virtual bool read(QIODevice & dev) {
        return true; // has nosize
    }

    virtual bool write(QIODevice & dev) {
        return dev.putChar(0); // Just marker
    }


    virtual QString toString() const {
        return QString("[undefined]");
    }

};

struct null_type : public Serializable
{
    virtual bool read(QIODevice & dev) {
        return true; // has nosize
    }

    virtual bool write(QIODevice & dev) {
        return dev.putChar(0x1); // Just marker
    }

    virtual QString toString() const {
        return QString("[null]");
    }

};

struct false_type : public Serializable
{
    virtual bool read(QIODevice & dev) {
        return true; // has nosize
    }

    virtual bool write(QIODevice & dev) {
        return dev.putChar(0x2); // Just marker
    }

    virtual QString toString() const {
        return QString("[false]");
    }

};

struct true_type : public Serializable
{
    virtual bool read(QIODevice & dev) {
        return true; // has nosize
    }

    virtual bool write(QIODevice & dev) {
        return dev.putChar(0x3); // Just marker
    }

    virtual QString toString() const {
        return QString("[true]");
    }

};

struct integer_type : public U29
{
    virtual bool read(QIODevice & dev) {
        return U29::read(dev);
    }

    virtual bool write(QIODevice &dev) {
        return U29::write(dev);
    }

    virtual QString toString() const {
        return QString("[int:%1]").arg(U29::toString());
    }
};

struct string_type : public UTF_8_vr
{
    virtual bool read(QIODevice & dev) {
        return UTF_8_vr::read(dev);
    }

    virtual QString toString() const {
        return QString("[string:%1]").arg(UTF_8_vr::toString());
    }
};



struct array_type : public Serializable // TODO: sparse arrays
{
    QVector<Serializable*> data;
    QMap<UTF_8_vr, Serializable*> assoc; // assoc part

    virtual bool read(QIODevice & dev) {
        U29 cnt;
        if (!cnt.read(dev))
            return false;

        qDeleteAll(data);
        qDeleteAll(assoc);
        data.clear();
        assoc.clear();

        if (cnt.m_value & 0x1) { // Normal array
            int siz = cnt.m_value >> 1;
            data.reserve(siz); // size of dense part
            UTF_8_vr v;
            while (v.read(dev)) { // fill assoc
                if (v.value.isEmpty()) // If empty -> this was last element of assoc. array
                    break;
                assoc[v] = parse_value(dev); // parse assoc value
            }
            for (int i=0; i<siz; i++) { // fill dense
                data.append(parse_value(dev));
            }
        } else { // ref
            return false;
        }


        return true;
    }

    virtual QString toString() const {
        QString v = QString("[array[%1]: ").arg(data.size());
        Serializable * s;
        if (assoc.size()) {
            foreach (UTF_8_vr key, assoc.keys()) { //semi optimal
                v.append("{ ").append(key.toString()).append(" : ").append(assoc[key]->toString()).append("}");
            }
        }
        foreach(s, data)
            v.append(s->toString()).append(", ");
        v.append("]");
        return v;
    }
};
} // namespace
#endif // AMF3VARS_H
