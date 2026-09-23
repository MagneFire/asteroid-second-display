// SPDX-FileCopyrightText: 2023 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

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