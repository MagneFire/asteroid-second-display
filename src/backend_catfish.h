#ifndef BACKEND_CATFISH_H
#define BACKEND_CATFISH_H

#include "backend.h"

namespace AsteroidOS::SecondDisplayDaemon
{
class CatfishBackend : public Backend
{
   public:
    CatfishBackend();
    virtual ~CatfishBackend();
    virtual bool HasSecondDisplay() { return true; };
    virtual int SynchronizeTime();
    virtual bool HasTimepieceMode() { return true; }
    virtual int EnterTimepieceMode();
    virtual bool HasStepCounter() { return true; }
    virtual int SetStepCounter(bool enable);

   private:
    void* OpenLibrary();
    void LoadSymbols();
    void* LoadSymbol(const char* symbol_string);
    int CloseLibrary(void* lib_mcutool);
    void* mcutool_handle;
};
};      // namespace AsteroidOS::SecondDisplayDaemon
#endif  // BACKEND_CATFISH_H