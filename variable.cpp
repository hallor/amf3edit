#include <auto_ptr.h>
#include "variable.h"
#include "parser.h"
#include "amf3vars.h"
#include "readexception.h"

using amf3::UTF_8_vr;
using std::auto_ptr;

Variable::Variable() : m_name(NULL), m_value(NULL)
{
}

Variable::~Variable()
{
    delete m_value;
    m_value = NULL;
    delete m_name;
    m_name = NULL;
}

void Variable::read(QIODevice &dev, const Parser &parser)
{
    if (!m_name)
        m_name = new amf3::UTF_8_vr();

    m_name->read(dev);

    if (m_value)
        delete m_value;

    m_value = parser.read(dev);

    if (!m_value)
        throw ReadException(dev, "Failed to parse value.");

    char x = 0;
    // TODO: why that?
    if (dev.getChar(&x) < 0) // One byte pad after key-value pair
        throw ReadException(dev);

    if (x!=0) // This should not happen
        throw ReadException("WTF");
}

QString Variable::toString() const
{
    return QString("<%1:%2>").arg(m_name.toString(), m_value->toString());
}

QString Variable::name() const
{
    if (m_name)
        return m_name->value;
    return QString("??");
}
