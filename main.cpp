#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <stdio.h>


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


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int i;
    deserializeInt(testInt, i);
    printf("-> %i\n", i);

    serializeInt(0x34, ba);
    serializeInt(0x34a, ba);
    serializeInt(0x34bb, ba);
    serializeInt(0x34bbc, ba);
    serializeInt(0x34bbccd, ba);

    return 0;//a.exec();
}
