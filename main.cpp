#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <stdio.h>


char testInt[] = { 0x04, 0x89, 0x0c, 0x5, 0x3, 0x4};

QByteArray ba(QByteArray::fromRawData(testInt, sizeof(testInt)));

bool serializeInt(int from, QByteArray * to) {
    if (from > (2 << 29) - 1) // overflow
        return false;

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
    return 0;//a.exec();
}
