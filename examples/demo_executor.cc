#include <memory>
#include <future>
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

    std::future fut = std::async(std::launch::async, []
                                 { loquat::Epoll::GetInstance().Wait(); });

    spdlog::debug("start service");

    fut.get();
    return 0;
}