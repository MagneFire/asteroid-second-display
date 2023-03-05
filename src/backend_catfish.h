#ifndef BACKEND_CATFISH_H
#define BACKEND_CATFISH_H

#include "backend.h"

namespace AsteroidOS::SecondDisplayDaemon
{
class CatfishBackend : public Backend
{
   public:
    CatfishBackend(){};
    virtual ~CatfishBackend(){};
    virtual int SynchronizeTime();
};
};      // namespace AsteroidOS::SecondDisplayDaemon
#endif  // BACKEND_CATFISH_H