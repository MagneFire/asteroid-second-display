#ifndef SECOND_DISPLAY_H
#define SECOND_DISPLAY_H

#include <QObject>
#include "display_interface.h"

class SecondDisplay : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasSecondDisplay READ hasSecondDisplay)
    Q_PROPERTY(bool hasStepCounter READ hasStepCounter)
    Q_PROPERTY(bool stepCounter READ getStepCounter WRITE setStepCounter NOTIFY stepCounterChanged)

   public:
    SecondDisplay(QObject *parent = 0);
    ~SecondDisplay();

    bool hasSecondDisplay() const;
    Q_INVOKABLE bool synchronizeTime() const;
    bool hasStepCounter() const;
    bool getStepCounter() const;
    bool setStepCounter(bool enabled) const;

   signals:
    void stepCounterChanged();

   private:
    bool hasConnection() const;
    org::asteroid::display *display;
};

#endif  // SECOND_DISPLAY_H