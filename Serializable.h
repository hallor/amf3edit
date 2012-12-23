#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

class QIODevice;
class Serializable {
public:
    virtual ~Serializable() {}
    virtual void write(QIODevice & /*dev*/) const  { }
    virtual void read(QIODevice & /*dev*/) = 0;
    virtual QString toString() const {
        return QString("[serializable]");
    }
};

#endif // SERIALIZABLE_H
