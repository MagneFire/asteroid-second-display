#include <QObject>

class Hands: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.asteroid.hands")
public slots:
    virtual bool HasHands();
    virtual int SetWatchMode(bool enable);
    virtual int MoveHands(int, int);
    virtual int MoveAllHands(int);
    virtual int Calibrate(int, int);
};
