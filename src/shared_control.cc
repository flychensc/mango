#pragma once

#include "shared_control.h"

namespace mango
{
    std::atomic<bool> SharedControl::isRunning{false};
}
