#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

class Serializable {
public:
    virtual void write(QIODevice & /*dev*/) const throw() { }
    virtual void read(QIODevice & dev) throw() = 0;
    virtual QString toString() const {
        return QString("[serializable]");
    }
};

#endif // SERIALIZABLE_H
