#ifndef READEXCEPTION_H
#define READEXCEPTION_H
#include <stdexcept>
#include <cstdio>
#include <QString>
#include <QIODevice>

#define MAXLEN 256
class ReadException : public std::exception
{
public:
    explicit ReadException(int where, const char * msg) {
        snprintf(msg, MAXLEN, "ReadException at %d (%x): %s", where, where, msg);
    }

    explicit ReadException(const QIODevice & dev, const char * msg) {
        snprintf(msg, MAXLEN, "ReadException at %d (%x): %s", dev.pos(), dev.pos(), msg);
    }

    explicit ReadException(const QIODevice & dev) {
        snprintf(msg, MAXLEN, "ReadException at %d (%x): %s", dev.pos(), dev.pos(), dev.errorString().toAscii().constData());
    }

    virtual const char * what() {
        return msg;
    }
private:
    char msg[MAXLEN];
};

class WriteException : public std::exception
{
public:
    explicit WriteException(int where, const char * msg) {
        snprintf(msg, MAXLEN, "WriteException at %d (%x): %s", where, where, msg);
    }

    explicit WriteException(const QIODevice & dev, const char * msg) {
        snprintf(msg, MAXLEN, "WriteException at %d (%x): %s", dev.pos(), dev.pos(), msg);
    }

    explicit WriteException(const QIODevice & dev) {
        snprintf(msg, MAXLEN, "WriteException at %d (%x): %s", dev.pos(), dev.pos(), dev.errorString().toAscii().constData());
    }

    virtual const char * what() {
        return msg;
    }
private:
    char msg[MAXLEN];
};


#endif // READEXCEPTION_H
