#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <stdio.h>
#include <stdexcept>


char testInt[] = { 0x04, 0xFF, 0xB0, 0x5, 0x3, 0x4};

QByteArray ba(QByteArray::fromRawData(testInt, sizeof(testInt)));


bool serializeInt(int from, QByteArray & to) {
    if (from > (2 << 29) - 1) // overflow
        return false;

    int pos = 0;

    while ((from & 0xFF000000) == 0){ // skip empty bytes
        from <<=8;
        pos ++;
    }
    printf("%08x -> Mam %d bajtow\n", from, 4 - pos);
    return true;

    while (from > 0 || pos < 4) {
        unsigned char a = from & 0xFF000000 >> 24;
        if (a > 0x7F) { // Overflow
            a = (a >> 1) | 0x80;
            to.append(a);
            from <<= 7;
        } else { // normal number
            to.append(a);
            from <<= 8;
        }
    }


    return false;
}

bool deserializeInt(const QByteArray & from, int & to) {
    to=0;

    if (from.at(0) != 0x4)
        return false;

    for (int i=0; i<4; i++) { // up to 4 bytes
        int c = (unsigned char)from.at(i+1);

        if (i != 3) {
            to = to << 7;
            to |= c & ~0x80;
        }
        else { // Last char has 8 bits
            to = to << 8;
            to |= c;
        }

        if ( (c & 0x80) == 0 )  // End of number
            break;
    }

    if (to > 0x3fffFFFF)
        return false;
    return true;
}

class Item {
public:
    virtual quint8 code()  { return 0;} // Code of item
    virtual quint32 size() = 0;
    virtual bool read(QIODevice & dev) = 0;
    virtual bool write(QIODevice & dev) = 0;
};

// Utf8 String used as keys
struct UtfKey : public Item
{ // Format: quint16 len | flag, name -> utf8
    QString name;
    bool flag;

    virtual quint32 size() { return name.toUtf8().size() + 2; }

    virtual bool read(QIODevice & dev)
    {
        quint8 len;
        dev.read((char*)&len, sizeof(len));
        flag = (len & 0x1) == 1;
        len >>= 1;
        if (!flag)
            throw std::runtime_error("Missing bit flag");
        name = dev.read(len);
        return true;
    }

    virtual bool write(QIODevice & dev)
    {
        QByteArray bin_name;
        bin_name = name.toUtf8();
        quint8 len = bin_name.length();
        len <<= 1;
        if (flag)
            len |= 1;
        dev.write((char*)len, sizeof(len));
        dev.write(bin_name);
        return true;
    }
};

struct Header : public Item {
    bool big_endian;
    quint32 file_size;
    QString root_name;
    quint8 version;
    static const quint8 sign_valid[10];

    quint8 code() { return 0; } // Code has no sense for Headers

    quint32 size() { // Size of header (on disk)
        return /*6 +*/ 10 + 2 + root_name.length() + 3 + 1;
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
        printf("Version: %x\n", version);
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

struct value : public Item {
    UtfKey key; // doczepiony
    quint8 kod; // na przyczepke
    int pad; // liczba pad'ow

    quint32 size() { return key.size() + 1; }

    bool read(QIODevice & dev)
    {
        if (! readInt(dev))
            return false;

        pad = 0;
        char b=0;
        while (b == 0)
        {
            dev.read(&b, 1);
            pad++;
        }
        dev.ungetChar(b);
        return true;
    }

    bool write(QIODevice & dev)
    {
        if (!writeInt(dev))
            return false;
        char b=0;
        for (int i=0; i<pad; i++) // Pad data
            dev.write(&b, 1);
        return true;
    }

    virtual bool readInt(QIODevice & dev)
    {
        dev.read((char*)&kod, 1);
        return kod == code();
    }

    virtual bool writeInt(QIODevice & dev)
    {
        dev.write((char*)&kod, 1);
        return true;
    }

    virtual QString stringify() {
        return QString("[%1:%3]").arg(key.name);
    }
};

struct value_undefined : public value {
    quint8 code() { return 0x0; }

    QString stringify() {
        return value::stringify().arg("undefined");
    }
};

struct value_null : public value {
    quint8 code() { return 0x1; }

    QString stringify() {
        return value::stringify().arg("null");
    }

};

struct value_false : public value {
    quint8 code() { return 0x2; }
    QString stringify() {
        return value::stringify().arg("false");
    }
};

struct value_true : public value {
    quint8 code() { return 0x3; }
    QString stringify() {
        return value::stringify().arg("true");
    }
};

struct value_int : public value {
    int num;

    value_int() : value(), num(42) {}

    quint8 code() { return 0x4; }
    QString stringify() {
        return value::stringify().arg("int") + QString("= %1").arg(num);
    }
};


value * read_value(QIODevice & dev)
{
    value * ret = NULL;
    UtfKey k;
    if (!k.read(dev))
        return ret;

    quint8 code;
    dev.peek((char*)&code, 1);

    switch (code)
    {
    case 0x00: ret = new value_undefined(); break;
    case 0x01: ret = new value_null(); break;
    case 0x02: ret = new value_false(); break;
    case 0x03: ret = new value_true(); break;
    case 0x04: ret = new value_int(); break;
    default:
        return ret;
    }
    ret->key = k;
    ret->read(dev);
    return ret;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int i;
    deserializeInt(testInt, i);
    printf("-> %i\n", i);

    serializeInt(0x34, ba);

    QFile in("jacksmith_1.sol"),
          out("test.sol");
    in.open(QIODevice::ReadOnly);
    out.open(QIODevice::WriteOnly | QIODevice::Truncate);

    Header h;

    printf("-> %d\n", h.read(in));

    printf("Loaded %d bytes. Root name %s.\n", h.file_size, h.root_name.toAscii().constData());

    value * v;
    while ( (v = read_value(in)) != NULL)
        printf("-> %s\n", v->stringify().toAscii().constData());

//    UtfKey k;
//    k.read(in);

//    printf("Read key: %s\n", k.name.toAscii().constBegin());

    printf("<- %d\n", h.write(out));

    return 0;//a.exec();
}
