#ifndef HANDS_H
#define HANDS_H

#include <QObject>
#include "hands_interface.h"

class Hands : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasHands READ hasHands)
   public:
    Hands(QObject *parent = 0);
    ~Hands();

    bool hasHands() const;

   private:
    org::asteroid::hands *hands;
};

#endif  // HANDS_H