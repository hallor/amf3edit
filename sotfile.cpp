#include <QDataStream>
#include <memory>

#include "sotfile.h"
#include "exception.h"
#include "Variable.h"
#include "amf3parser.h"
#include "amfparser.h"

namespace {
const quint8 sign_valid[] ={'T','C','S', 'O', 0, 4, 0, 0, 0, 0};

const Parser * parsers[] = { new AmfParser(), new Amf3Parser() };
}

SotFile::SotFile() : m_rootName("Unknown"), m_version(-1), m_bigEndian(true)
{ }

SotFile::~SotFile()
{
    qDeleteAll(m_data);
    m_data.clear();
}

// TODO: nice handle of >> reads
void SotFile::load(QIODevice & from)
{
    if (m_data.count()) {
        qDeleteAll(m_data);
        m_data.clear();
    }


    QDataStream str(&from); // by default big endian mode
    quint16 endi = 0;
    str >> endi;
    m_bigEndian = endi == 0x00BF;

    if (! m_bigEndian)
        throw ReadException(from, "Only big endian mode supported.");

    int file_size;
    str >> file_size;
    if (file_size + 6 != from.size()) // to fit rest of data
        throw ReadException(from, "Declared file size differs from real one.");

    char sign[10];
    if (str.readRawData(sign, sizeof(sign)) < 0)
        throw ReadException(from);

    if (memcmp(sign, sign_valid, sizeof(sign))) // invalid signature
        throw ReadException(from, "TCSO signature not found.");

    quint16 root_len = 0;
    str >> root_len;
    {
        char name[root_len];
        if (str.readRawData(name, root_len) < 0)
            throw ReadException(from);
        m_rootName = QString::fromUtf8(name, root_len);
    }
    if (str.skipRawData(3) < 0 ) // 3 bytes of crap
        throw ReadException(from);

    str >> m_version;

    if (m_version !=3) // Support only amfv3
        throw ReadException(from, "Unsupported AMF version");

    const Parser * p;
    if (m_version == 3)
        p = parsers[1]; // TODO

    while (! str.atEnd()) {
        Variable * v;

        v = p->readVariable(from);
        if (v)
            m_data.push_back(v);
    }
}

QString SotFile::toString() const
{
    QString v;

    v = QString("[RootName: '%1']\n[Version: %2]\n").arg(m_rootName).arg(m_version);

    foreach(Variable * var, m_data)
        v.append(var->toString());

    return v;
}

void SotFile::save(QIODevice & /*to*/) const
{

    throw std::logic_error("Not implemented");
#if 0
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
#endif
}
