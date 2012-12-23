#ifndef AMF3PARSER_H
#define AMF3PARSER_H
#include "Serializable.h"

struct U29 : public Serializable {

    int value;

    U29() : value(42) {}

    bool read(QIODevice & dev)
    {
        value=0;

        int i;
        for (i=0; i<4; i++) { // up num 4 bytes
            quint8 c;
            if (!dev.read((char*)&c, 1))
                return false;

            if (i != 3) {
                value = value << 7;
                value |= c & ~0x80;
            }
            else { // Last char has 8 bits
                value = value << 8;
                value |= c;
            }

            if ( (c & 0x80) == 0 )  // End of number
                break;
        }
        if (value > 0x3fffFFFF)
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
        return QString("[U29:%1]").arg(value);
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
        if (m.value & 0x1) { // value
            ref = -1;
            value = dev.read(m.value >> 1);
            if (dev.atEnd() && value.length() == 0) // detect EOF
                return false;
        } else { // ref
            ref = m.value >> 1;
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

        if (cnt.value & 0x1) { // Normal array
            int siz = cnt.value >> 1;
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

class Amf3Parser
{
public:
    Amf3Parser();
    Serializable * parse_value(QIODevice & dev)
    {
        Serializable * ret = NULL;
        quint8 code;
        quint64 start_pos = dev.pos();
        if (!dev.read((char*)&code, 1))
            return NULL;
        switch (code)
        {
        case 0x00: ret = new undefined_type(); break;
        case 0x01: ret = new null_type(); break;
        case 0x02: ret = new false_type(); break;
        case 0x03: ret = new true_type(); break;
        case 0x04: ret = new integer_type(); break;
        case 0x06: ret = new string_type(); break;
        case 0x09: ret = new array_type(); break;
        default:
            return ret;
        }
        if (ret)
            if (ret->read(dev))
                return ret;
        delete ret;
        printf("Error reading token %X @%X, failed @%X\n", code, start_pos, dev.pos());
        return NULL;
    }
};

#endif // AMF3PARSER_H
