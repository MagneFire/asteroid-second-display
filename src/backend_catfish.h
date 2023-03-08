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
    typedef struct nativeFunctions_t {
        int (*autoLowPowerScreen)(int32_t, int32_t, int32_t enable);
        int (*bandMode)();
        int (*cutOffScreen)();
        int (*enableHeartRate)(int32_t, int32_t, int32_t enable);
        int (*enableLowPowerScreen)(int32_t, int32_t, int32_t enable);
        int (*enableMotion)(int32_t, int32_t, int32_t enable);
        int (*enableStepCounter)(int32_t, int32_t, int32_t enable);
        int (*getBandModeData)();
        int (*getDataVersion)();
        int (*syncSteps)(int32_t, int32_t, int32_t steps);
        int (*syncTime)();
        int (*updateFitnessState)(int32_t, int32_t, int32_t type, int32_t state, int32_t heartRate, int64_t duration,
                                  int32_t calorie, float distance, int32_t speed, int32_t gpsStatus,
                                  uint8_t isKilometer);
        int (*wipeBandModeData)();
    } nativeFunctions_t;

    nativeFunctions_t nativeFunctions;
};
};      // namespace AsteroidOS::SecondDisplayDaemon
#endif  // BACKEND_CATFISH_H