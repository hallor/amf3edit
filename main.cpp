#include <QtCore/QCoreApplication>
#include <QFile>
#include <stdio.h>
#include <stdexcept>

#include "sotfile.h"

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

    SotFile f;

    try {
        f.load(in);
    } catch (std::exception & e) {
        printf("%s\n", e.what);
    }

    printf("%s\n", f.toString().toAscii().constData());
    printf("Finished @%X\n", in.pos());

    return 0;//a.exec();
}
