#pragma once

#include <memory>
#include "builder.h"

namespace mango
{
    class Director
    {
    public:
        void setBuilder(std::shared_ptr<Builder> b)
        {
            builder = b;
        }

        void construct()
        {
            builder->joinToEpoll();
        }

    private:
        std::shared_ptr<Builder> builder;
    };
}
