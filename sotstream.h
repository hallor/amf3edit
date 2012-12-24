#ifndef SOTSTREAM_H
#define SOTSTREAM_H

#include <QFile>

class SotStream : public QFile
{
    Q_OBJECT
public:
    explicit SotStream(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SOTSTREAM_H
