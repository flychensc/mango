#include <memory>
#include <spdlog/spdlog.h>
#include "executor.h"

using namespace mango;

int main(int argc, char *argv[], char *envp[])
{
    spdlog::set_level(spdlog::level::debug);

    std::shared_ptr<ExecutorService> p_service = std::make_shared<ExecutorService>("127.0.0.1", 2012);
    p_service->start();

    spdlog::debug("start service");

    return 0;
}