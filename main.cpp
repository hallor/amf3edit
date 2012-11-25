#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <stdio.h>
#include <stdexcept>
#include <QVector>

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
            dev.read((char*)&c, 1);

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

    QString toString() const {
        return QString("[U29:%1]").arg(value);
    }

};

struct UTF_8_vr : public Serializable
{
    int ref;
    QString value;

    bool read(QIODevice & dev)
    {
        U29 m;
        if (!m.read(dev))
            return false;
        if (m.value & 0x1) { // value
            ref = -1;
            value = dev.read(m.value >> 1);
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

    virtual QString toString() const {
        return QString("[undefined]");
    }

};

struct null_type : public Serializable
{
    virtual bool read(QIODevice & dev) {
        return true; // has nosize
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

    virtual QString toString() const {
        return QString("[false]");
    }

};

struct true_type : public Serializable
{
    virtual bool read(QIODevice & dev) {
        return true; // has nosize
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

    virtual bool read(QIODevice & dev) {
        U29 cnt;
        if (!cnt.read(dev))
            return false;

        qDeleteAll(data);
        data.clear();
        if (cnt.value & 0x1) { // Normal array
            int siz = cnt.value >> 1;
            data.reserve(siz);
            dev.read(1); // TODO: utf-8-empty -> assume assoc array empty
            for (int i=0; i<siz; i++) { // fill data
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

    virtual bool read(QIODevice & dev) {
        if (!name.read(dev))
            return false;
        value = parse_value(dev); // TODO: mem mgmt
        if (!value)
            return false;
        if (dev.pos() % 2) {// padding?
            char c;
            dev.getChar(&c);
            if (c != 0)
                return false;
        }
        return true;
    }

    virtual QString toString() const {
        return QString("<%1:%2>").arg(name.toString(), value->toString());
    }
};

#if 0
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

bool deserializeInt(QIODevice & from, int & num) {
    num=0;

    int i;
    for (i=0; i<4; i++) { // up num 4 bytes
        quint8 c;
        from.read((char*)&c, 1);

        if (i != 3) {
            num = num << 7;
            num |= c & ~0x80;
        }
        else { // Last char has 8 bits
            num = num << 8;
            num |= c;
        }

        if ( (c & 0x80) == 0 )  // End of number
            break;
    }
//    num_bytes = i+1;

    if (num > 0x3fffFFFF)
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
struct UtfString : public Item
{ // Format: quint16 len | flag, name -> utf8
    QString name;
    int ref_id; // if not, then reference

    virtual quint32 size() { return (ref_id >=0 ? 1 : name.toUtf8().size() + 2); } // TODO: handle reference size properly

    virtual bool read(QIODevice & dev)
    {
        int len;
        if (!deserializeInt(dev, len)) // get length
            return false;
        name.clear();
        ref_id = -1; // reference has only 28 bits

        if (len & 0x1){ // Normal string
            name = dev.read(len >> 1);
        } else { // reference
            ref_id = len >> 1;
        }
        return true;
    }

    virtual bool write(QIODevice & dev)
    {
        QByteArray bin_name;
        bin_name = name.toUtf8();
        quint8 len = bin_name.length();
        len <<= 1;
        if (ref_id < 0) // Todo: handle of refs
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
    UtfString key; // doczepiony
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
    int num, num_bytes;

    value_int() : value(), num(42) {}

    quint8 code() { return 0x4; }

    quint32 size() { return value::size() + num_bytes; }


    bool readInt(QIODevice & dev)
    {
        if (! value::readInt(dev))
            return false;

        num=0;

        int i;
        for (i=0; i<4; i++) { // up num 4 bytes
            quint8 c;
            dev.read((char*)&c, 1);

            if (i != 3) {
                num = num << 7;
                num |= c & ~0x80;
            }
            else { // Last char has 8 bits
                num = num << 8;
                num |= c;
            }

            if ( (c & 0x80) == 0 )  // End of number
                break;
        }
        num_bytes = i+1;

        if (num > 0x3fffFFFF)
            return false;
        return true;
    }

    bool writeInt(QIODevice & dev)
    {
        if (!value::writeInt(dev))
            return false;

        int val = num;

        if (val > (2 << 29) - 1) // overflow
            return false;

        int pos = 0;

        while ((val & 0xFF000000) == 0){ // skip empty bytes
            val <<=8;
            pos ++;
        }

//        printf("Mam val %08x, bajtow %d\n", val, 4 - pos);
        pos = (4 - pos); // Num of bits
//        quint8 v[pos];
        return false; // TODO

        for (int i=0; i<pos; i++) {
//            if (val < 0xFF)
        }

        while (pos > 0) {
            if (pos == 8) {// Last 8 bits
                quint8 tmp = val >> 24;
                dev.write((char*)&tmp, 1);
                pos = 0;
            } else  {
                if (pos > 7) { // More than 7 bits left
                    quint8 tmp = 0x80 | ( val >> 25 );
                    dev.write((char*)&tmp, 1);
                    val <<= 7;
                    pos -= 7;
                } else { // less than 7 bits left
                    val <<= 8;
                    pos -=8;
                }
            }
        }

        return true;
    }

    QString stringify() {
        return value::stringify().arg("int") + QString("= %1").arg(num);
    }
};

struct value_string : public value {
    UtfString s;

    quint8 code() { return 0x6; }

    quint32 size() { return value::size() + s.size(); }


    bool readInt(QIODevice & dev)
    {
        if (! value::readInt(dev))
            return false;

        return s.read(dev);
    }

    QString stringify() {
        if (s.ref_id >=0)
            return value::stringify().arg("string_ref") + QString("= @%1").arg(s.ref_id);
        else
            return value::stringify().arg("string") + QString("= %1").arg(s.name);
    }

};

struct value_array : public value {
    int msize;
    int ref_id;

    quint8 code() { return 0x6; }

    quint32 size() { return value::size() + msize; }


    bool readInt(QIODevice & dev) // TODO: handle of non-dense arrays
    {
        if (! value::readInt(dev))
            return false;
        int no;
        if (!deserializeInt(dev, no))
            return false;

        if (no & 0x1) { // dense
            msize = no >> 1;
            ref_id = -1;
            dev.read(msize);
        } else { // ref
            msize =0;
            ref_id = no >> 1;
        }


        dev.read(msize);
        return false;
    }

    QString stringify() {
        if (ref_id >=0)
            return value::stringify().arg("array_ref") + QString("= @%1").arg(ref_id);
        else
            return value::stringify().arg("array");
    }

};


value * read_value(QIODevice & dev)
{
    value * ret = NULL;
    UtfString k;
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
    case 0x06: ret = new value_string(); break;
    default:
        return ret;
    }
    ret->key = k;
    ret->read(dev);
    return ret;
}
#endif

Serializable * parse_value(QIODevice & dev)
{
    Serializable * ret = NULL;
    quint8 code;
    dev.read((char*)&code, 1);
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
    ret->read(dev);
    return ret;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    int i;
//    deserializeInt(testInt, i);
//    printf("-> %i\n", i);

//    serializeInt(0x34, ba);

    QFile in("jacksmith_1.sol"),
          out("test.sol");
    in.open(QIODevice::ReadOnly);
    out.open(QIODevice::WriteOnly | QIODevice::Truncate);

    in.seek(0x36);

    variable v;
    while (v.read(in)) {
        printf("%s\n", v.toString().toAscii().constData());
    }
#if 0
    string_type v;
    printf ("r: %d ", v.read(in));
    printf("%s\n", v.toString().toAscii().constData());

    Serializable * v2;
//    in.seek(0x1ea); // skip symbol for now
    v2 = parse_value(in);
    printf ("r: %p ", v2);
    if (v2)
        printf("%s\n", v2->toString().toAscii().constData());

    printf("pos: %x\n", in.pos());
    printf ("r: %d ", v.read(in));
    printf("%s\n", v.toString().toAscii().constData());
#endif
#if 0
    Header h;

    printf("-> %d\n", h.read(in));

    printf("Loaded %d bytes. Root name %s.\n", h.file_size, h.root_name.toAscii().constData());

    in.seek(0x103);
    value * v;
    while ( (v = read_value(in)) != NULL) {
        printf("-> %s\n", v->stringify().toAscii().constData());
//        v->write(out);
    }

//    UtfKey k;
//    k.read(in);

//    printf("Read key: %s\n", k.name.toAscii().constBegin());

//    printf("<- %d\n", h.write(out));
#endif
    return 0;//a.exec();
}
