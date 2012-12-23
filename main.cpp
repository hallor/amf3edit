#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <stdio.h>
#include <stdexcept>
#include <QVector>
#include <QMap>

#include "Serializable.h"

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
