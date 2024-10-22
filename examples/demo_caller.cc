#include <memory>
#include <spdlog/spdlog.h>
#include "caller_builder.h"
#include "director.h"

using namespace mango;

int main(int argc, char *argv[], char *envp[])
{
    spdlog::set_level(spdlog::level::debug);

    std::shared_ptr<Builder> builder = std::make_shared<CallerBuilder>("127.0.0.1", 2024);
    Director director;
    director.setBuilder(builder);

    director.construct();

    auto p_caller = std::dynamic_pointer_cast<Caller>(builder->getResult());

    p_caller->start();

    spdlog::debug("start caller");

    p_caller->Connect("127.0.0.1", 2012);

    spdlog::debug("caller connect");

    // p_caller->call();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    spdlog::debug("caller stop");

    p_caller->stop();
    return 0;
}