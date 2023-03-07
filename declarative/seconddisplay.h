#ifndef SECOND_DISPLAY_H
#define SECOND_DISPLAY_H

#include <QObject>
#include "display_interface.h"

class SecondDisplay : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasSecondDisplay READ hasSecondDisplay)
   public:
    SecondDisplay(QObject *parent = 0);
    ~SecondDisplay();

    bool hasSecondDisplay() const;
    Q_INVOKABLE bool synchronizeTime() const;

   private:
    org::asteroid::display *display;
};

#endif  // SECOND_DISPLAY_H