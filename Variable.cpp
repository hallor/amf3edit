#include <auto_ptr.h>
#include "Variable.h"
#include "Parser.h"
#include "amf3vars.h"
#include "exception.h"

using amf3::UTF_8_vr;
using std::auto_ptr;

Variable::Variable(const Parser & parser) : m_parser(parser), m_name(NULL), m_value(NULL)
{
}

Variable::~Variable()
{
    delete m_value;
    m_value = NULL;
    delete m_name;
    m_name = NULL;
}

void Variable::read(QIODevice &dev)
{
    if (!m_name)
        m_name = new amf3::UTF_8_vr();

    m_name->read(dev);

    if (m_value)
        delete m_value;

    m_value = m_parser.readValue(dev);

    if (!m_value)
        throw ReadException(dev, "Failed to parse value.");

    char x = 0;
    // TODO: why that?
    if (dev.getChar(&x) < 0) // One byte pad after key-value pair
        throw ReadException(dev);

    if (x!=0) // This should not happen
        throw ReadException(dev, "WTF");
}

QString Variable::toString() const
{
    return QString("<%1:%2>").arg(m_name->valueToString())
            .arg(m_value->toString());
}

bool Variable::isComplex()
{
    if (m_value)
        return m_value->isComplex();
    return false;
}

QString Variable::name() const
{
    if (m_name)
        return m_name->value();
    return QString("??");
}
