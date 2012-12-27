#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

class QIODevice;
class Serializable {
public:
    Serializable() : m_parent(NULL) {}
    virtual ~Serializable() {}
    virtual void write(QIODevice & /*dev*/) const  { }
    virtual void read(QIODevice & /*dev*/) = 0;
    virtual QString toString() const {
        return QString("[serializable]");
    }

    const Serializable * parent() const { return m_parent; }
    Serializable * parent() { return m_parent; }
    void setParent(Serializable * parent) { m_parent = parent; }
private:
    Serializable * m_parent;
};

#endif // SERIALIZABLE_H
