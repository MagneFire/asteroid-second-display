#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QDebug>

namespace AsteroidOS::SecondDisplayDaemon
{

class Hands : public QObject
{
   public:
    Hands(){};
    virtual int HasHands() { return false; };
    virtual int SetWatchMode(bool enable) { return false; };
    virtual int MoveHands(int, int) { return false; };
    virtual int MoveAllHands(int) { return false; };
    virtual int Calibrate(int, int) { return false; };
};

class Backend : public QObject
{
   public:
    Backend(){};
    virtual ~Backend(){};
    virtual bool HasSecondDisplay() { return false; };
    virtual int SynchronizeTime() { return 0; };
    virtual bool HasTimepieceMode() { return false; };
    virtual int EnterTimepieceMode() { return 0; };
    virtual bool HasStepCounter() { return false; };
    virtual int SetStepCounter(bool enable) { return 0; };
    virtual int HasHands() { return false; };
    virtual Hands* GetHands() { return &hands; };

    static Backend* Get();

   private:
    static Backend* GetBackend();
    static Backend* instance;
    Hands hands;
    bool state;
};
};      // namespace AsteroidOS::SecondDisplayDaemon
#endif  // BACKEND_H