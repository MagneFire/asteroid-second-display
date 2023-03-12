#include "backend_catfish.h"

#include <QDebug>

#include <iostream>
#include <dlfcn.h>
#include <hybris/common/dlfcn.h>
#include <hybris/properties/properties.h>
#include <MGConfItem>

using namespace AsteroidOS::SecondDisplayDaemon;

CatfishBackend::CatfishBackend()
{
    mcutool_handle = OpenLibrary();
    LoadSymbols();
}

CatfishBackend::~CatfishBackend()
{
    CloseLibrary(mcutool_handle);
}

void* CatfishBackend::OpenLibrary()
{
    auto lib_mcutool = hybris_dlopen("libmcutool.so", RTLD_LAZY);
    if (!lib_mcutool) {
        std::cerr << "Unable to load libmcutool.so" << std::endl;
        return nullptr;
    }
    return lib_mcutool;
}

void* CatfishBackend::LoadSymbol(const char* symbol_string)
{
    void* symbol = hybris_dlsym(mcutool_handle, symbol_string);
    if (!symbol) {
        std::cerr << "Unable to get symbol " << symbol_string << std::endl;
        return nullptr;
    }
    return symbol;
}

void CatfishBackend::LoadSymbols()
{
    if (!mcutool_handle) return;

    nativeFunctions.autoLowPowerScreen = (int (*)(int32_t, int32_t, int32_t))LoadSymbol(
        "Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeAutoLowPowerScreen");
    nativeFunctions.bandMode = (int (*)())LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeBandMode");
    nativeFunctions.cutOffScreen =
        (int (*)())LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeCutOffScreen");
    nativeFunctions.enableHeartRate = (int (*)(int32_t, int32_t, int32_t))LoadSymbol(
        "Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeEnableHeartRate");
    nativeFunctions.enableLowPowerScreen = (int (*)(int32_t, int32_t, int32_t))LoadSymbol(
        "Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeEnableLowPowerScreen");
    nativeFunctions.enableMotion = (int (*)(int32_t, int32_t, int32_t))LoadSymbol(
        "Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeEnableMotion");
    nativeFunctions.enableStepCounter = (int (*)(int32_t, int32_t, int32_t))LoadSymbol(
        "Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeEnableStepCounter");
    nativeFunctions.getBandModeData =
        (int (*)())LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeGetBandModeData");
    nativeFunctions.getDataVersion =
        (int (*)())LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeGetDataVersion");
    nativeFunctions.syncSteps = (int (*)(int32_t, int32_t, int32_t))LoadSymbol(
        "Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeSyncSteps");
    nativeFunctions.syncTime = (int (*)())LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeSyncTime");
    nativeFunctions.updateFitnessState =
        (int (*)(int32_t, int32_t, int32_t, int32_t, int32_t, int64_t, int32_t, float, int32_t, int32_t,
                 uint8_t))LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeUpdateFitnessState");
    nativeFunctions.wipeBandModeData =
        (int (*)())LoadSymbol("Java_com_mobvoi_ticwear_mcuservice_CoreService_nativeWipeBandModeData");
}

int CatfishBackend::CloseLibrary(void* lib_mcutool)
{
    if (hybris_dlclose(lib_mcutool)) {
        std::cerr << "Failed to safely close the library" << std::endl;
        return -1;
    }
    return 0;
}
int CatfishBackend::SynchronizeTime()
{
    auto use12h = new MGConfItem("/org/asteroidos/settings/use-12h-format");

    if (use12h->value(false).toBool()) {
        property_set("persist.sys.time_12_24", "12");
    } else {
        property_set("persist.sys.time_12_24", "24");
    }

    return nativeFunctions.syncTime();
}

int CatfishBackend::EnterTimepieceMode()
{
    int res;
    res = nativeFunctions.autoLowPowerScreen(0, 0, true);
    if (res) {
        std::cerr << "ERROR autoLowPowerScreen: " << res << std::endl;
    }
    res = nativeFunctions.autoLowPowerScreen(0, 0, false);
    if (res) {
        std::cerr << "ERROR autoLowPowerScreen: " << res << std::endl;
    }
    res = nativeFunctions.cutOffScreen();
    if (res) {
        std::cerr << "ERROR cutOffScreen: " << res << std::endl;
    }
    res = nativeFunctions.wipeBandModeData();
    if (res) {
        std::cerr << "ERROR wipeBandModeData: " << res << std::endl;
    }
    res = nativeFunctions.bandMode();
    if (res) {
        std::cerr << "ERROR bandMode: " << res << std::endl;
    }
    return 1;
}

int CatfishBackend::SetStepCounter(bool enable)
{
    if (m_stepCounterEnabled == enable) return 0;

    m_stepCounterEnabled = enable;

    emit Backend::StepCounterChanged();

    return nativeFunctions.enableStepCounter(0, 0, enable);
}