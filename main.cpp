#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <stdio.h>
#include <stdexcept>
#include <QVector>
#include <QMap>

char testInt[] = { 0x04, 0xFF, 0xB0, 0x5, 0x3, 0x4};

QByteArray ba(QByteArray::fromRawData(testInt, sizeof(testInt)));

class Serializable {
public:
    virtual bool write(QIODevice & /*dev*/) { return false; }
    virtual bool read(QIODevice & dev) = 0;
    virtual QString toString() const {
        return QString("[serializable]");
    }
};

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
// fwd decl
Serializable * parse_value(QIODevice & dev);

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

struct variable : public Serializable
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


struct Header : public Serializable {
    bool big_endian;
    quint32 file_size;
    QString root_name;
    quint8 version;
    static const quint8 sign_valid[10];

    virtual QString toString() const {
        QString v;

        v = QString("[RootName: '%1']\n[Version: %2]\n").arg(root_name).arg(version);

        return v;
    }


    bool read(QIODevice & dev) {
        QDataStream str(&dev); // by default big endian mode
        quint16 endi = 0;
        str >> endi;
        big_endian = endi == 0x00BF;
        if (! big_endian)
            throw std::runtime_error("Invalid endiannes.");

        str >> file_size;
        if (file_size + 6 != dev.size()) // to fit rest of data
            throw std::runtime_error("File size invalid.");

        char sign[10];
        str.readRawData(sign, sizeof(sign));

        if (memcmp(sign, sign_valid, sizeof(sign))) // invalid signature
            throw std::runtime_error("No valid signature found.");

        quint16 root_len = 0;
        str >> root_len;
        {
            char name[root_len];
            str.readRawData(name, root_len);
            root_name = QString::fromUtf8(name, root_len);
        }
        str.skipRawData(3); // 3 bytes of crap
        str >> version;
        if (version !=3) // Support only amfv3
            throw std::runtime_error("Unsupported version of AMF");
        return str.status() == QDataStream::Ok;
    }

    bool write(QIODevice & dev) {
        QDataStream str(&dev); // by default big endian mode

        if (big_endian)
            str << (quint16)0x00BF;
        else
            str << (quint16)0xBF00;

        str << (quint32)file_size;
        str.writeRawData((char*)sign_valid, sizeof(sign_valid));
        QByteArray strUtf(root_name.toUtf8());
        str << (quint16) strUtf.size();
        str.writeRawData(strUtf.constData(), strUtf.size());

        str << (quint32) 0x00000003; // 3-byte padding + version
        return str.status() == QDataStream::Ok;
    }
};
const quint8 Header::sign_valid[] ={'T','C','S', 'O', 0, 4, 0, 0, 0, 0};


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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QFile in;

    if (argc > 1)
        in.setFileName(argv[1]);
    else
        in.setFileName("jacksmith_1.sol");

    QFile out("test.sol");
    in.open(QIODevice::ReadOnly);
    out.open(QIODevice::WriteOnly | QIODevice::Truncate);

    Header h;

    printf("Reading %s...\n", in.fileName().toAscii().constData());
    h.read(in);
    printf("%s", h.toString().toAscii().constData());

    variable v;
    while (v.read(in)) {
        printf("%s\n", v.toString().toAscii().constData());
    }
    printf("Finished @%X\n", in.pos());

    return 0;//a.exec();
}
