#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <stdio.h>
#include <stdexcept>


char testInt[] = { 0x04, 0xFF, 0xB0, 0x5, 0x3, 0x4};

QByteArray ba(QByteArray::fromRawData(testInt, sizeof(testInt)));

struct Header {
    bool big_endian;
    quint32 file_size;
    QString root_name;
    quint8 version;

    bool readHeader(QIODevice & dev) {
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
        static const char sign_valid[10]={'T','C','S', 'O', 0, 4, 0, 0, 0, 0};
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

    bool writeHeader(QIODevice * dev) {
        return true;
    }
};

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


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int i;
    deserializeInt(testInt, i);
    printf("-> %i\n", i);

    serializeInt(0x34, ba);

    QFile in("jacksmith_1.sol"), out("test.sol");
    in.open(QIODevice::ReadOnly);
    out.open(QIODevice::WriteOnly);

    Header h;

    printf("%d\n", h.readHeader(in));

    printf("Loaded %d bytes. Root name %s.\n", h.file_size, h.root_name.toAscii().constData());

    return 0;//a.exec();
}
