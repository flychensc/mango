#include <memory>
#include <spdlog/spdlog.h>
#include "executor_builder.h"
#include "director.h"
#include "message_creator.h"
#include "demo_message.h"

using namespace mango;

int main(int argc, char *argv[], char *envp[])
{
    spdlog::set_level(spdlog::level::debug);

    MessageCreator::registerMessageType(PING, createPingMessage);
    MessageCreator::registerMessageType(PONG, createPongMessage);

    std::shared_ptr<Builder> builder = std::make_shared<ExecutorServiceBuilder>("127.0.0.1", 2012);
    Director director;
    director.setBuilder(builder);

    director.construct();

    auto p_service = std::dynamic_pointer_cast<ExecutorService>(builder->getResult());

    p_service->start();
    spdlog::debug("start service");

    p_service->wait();
    return 0;
}