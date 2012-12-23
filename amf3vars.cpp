#include <stdexcept>
#include <QIODevice>

#include "amf3vars.h"
#include "Parser.h"
#include "exception.h"
using namespace amf3;
/////////////////////////////////////////////// U29
U29::U29() : m_value(42) {}

void U29::read(QIODevice & dev)
{
    m_value=0;

    int i;
    for (i=0; i<4; i++) { // up num 4 bytes
        quint8 c;
        if (dev.read((char*)&c, 1) < 1)
            throw ReadException(dev);

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
        throw std::logic_error("Integer overflow");
}

void U29::write(QIODevice & /*dev*/) const { // TODO: refactor
    throw std::runtime_error("Not implemented");
}

////////////////////////////////////////////// STRING
//TODO: refcache
bool UTF_8_vr::operator<(const UTF_8_vr & other) const {
    if (ref >=0 )
        return ref < other.ref;
    else
        return m_value < other.m_value;
}

void UTF_8_vr::read(QIODevice & dev)
{
    U29 m;

    m.read(dev);

    if (m.value() & 0x1) { // value
        ref = -1;
        m_value = dev.read(m.value() >> 1);
        if (dev.atEnd() && m_value.length() == 0) // detect EOF
            throw ReadException(dev);
    } else { // ref
        ref = m.value() >> 1;
    }
}

///////////////////////////////////////////////// INT
void integer_type::write(QIODevice &dev) const
{
    char d = 0x4;
    if (dev.write(&d, 1) < 1)
        throw WriteException(dev);
    U29::write(dev);
}


//////////////////////////////////////////////// ARRAY
void array_type::read(QIODevice &dev)
{
    U29 cnt;

    cnt.read(dev);

    qDeleteAll(data);
    qDeleteAll(assoc);
    data.clear();
    assoc.clear();

    if (cnt.value() & 0x1) { // Normal array
        int siz = cnt.value() >> 1;
        data.reserve(siz); // size of dense part
        UTF_8_vr v;
        while (!dev.atEnd()) { // fill assoc
            v.read(dev);
            if (v.value().isEmpty()) // If empty -> this was last element of assoc. array
                break;
            assoc[v] = parser.readValue(dev); // parse assoc value
        }
        for (int i=0; i<siz; i++) { // fill dense
            data.append(parser.readValue(dev));
        }
    } else { // ref
        throw ReadException(dev, "Referenced arrays not supported.");
    }
}

QString array_type::toString() const
{
    QString v = QString("[array[%1]:{").arg(data.size());
    Value * s;
    if (assoc.size()) {
        foreach (UTF_8_vr key, assoc.keys()) { //semi optimal
            v.append("{").append(key.toString()).append(" : ").append(assoc[key]->toString()).append("}");
        }
    }
    foreach(s, data)
        v.append(s->toString()).append(", ");
    v.append("}]");
    return v;
}


