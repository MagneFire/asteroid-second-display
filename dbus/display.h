#include <QObject>

class Display: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.asteroid.display")
public slots:
    virtual bool HasSecondDisplay();
    virtual int SynchronizeTime();
    virtual bool HasTimepieceMode();
    virtual int EnterTimepieceMode();
    virtual bool HasStepCounter();
    virtual int SetStepCounter(bool enable);
    virtual int HasHands();
};
