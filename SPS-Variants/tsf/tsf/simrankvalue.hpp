#ifndef __SIMRANKVALUE_H__
#define __SIMRANKVALUE_H__

class SimRankValue {
public:
    SimRankValue() : vid(-1), value(0.0) {}

    SimRankValue(int id, double v) : vid(id), value(v) {}

    int getVid() const { return vid; }

    double getValue() const { return value; }

    void setVid(int v) { vid = v; }

    void setValue(double val) { value = val; }

    void incValue(double delta) { value += delta; }

    void div(double base) { value /= base; }

    bool operator<(const SimRankValue &other) const {
        return value > other.value;
    }

private:
    int vid;
    double value;
};

#endif
