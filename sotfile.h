#ifndef SOTFILE_H
#define SOTFILE_H
#include <QString>
#include <QList>

class Variable;
class QIODevice;
class SotFile
{
public:
    SotFile();
    ~SotFile();

    void load(QIODevice & from) throw();
    void save(QIODevice & to) const throw();

    QString rootName() const { return m_rootName; }
    int version() const { return m_version; }
    bool bigEndian() const { return m_bigEndian; }

    QString toString() const;

    // Editable tree, variables have one instance and should be not removed for now. Class owns data.
    QList<Variable*> getTree() { return m_data; }
private:
    QString m_rootName;
    int m_version;
    bool m_bigEndian;
    QList<Variable*> m_data;
};

#endif // SOTFILE_H
