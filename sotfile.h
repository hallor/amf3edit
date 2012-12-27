#ifndef SOTFILE_H
#define SOTFILE_H
#include <QString>
#include <QVector>
#include <Serializable.h>

class Variable;
class QIODevice;
class SotFile : public Serializable
{
public:
    SotFile();
    ~SotFile();

    void read(QIODevice & from) ;
    void write(QIODevice & to) const ;

    QString rootName() const { return m_rootName; }
    int version() const { return m_version; }
    bool bigEndian() const { return m_bigEndian; }

    QString toString() const;

    // Editable tree, variables have one instance and should be not removed for now. Class owns data.
    QVector<Variable*> getTree() { return m_data; }
private:
    QString m_rootName;
    quint8 m_version;
    bool m_bigEndian;
    QVector<Variable*> m_data;
};

#endif // SOTFILE_H
