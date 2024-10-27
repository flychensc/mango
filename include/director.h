#pragma once

#include <memory>
#include "builder.h"

namespace mango
{
    class Director
    {
    public:
        /**
         * @brief Set the RPC role builder
         * @param b RPC role builder
         */
        void setBuilder(std::shared_ptr<Builder> b)
        {
            builder = b;
        }

        /**
         * @brief Construct RPC role
         */
        void construct()
        {
            builder->joinToEpoll();
        }

    private:
        std::shared_ptr<Builder> builder;
    };
}
