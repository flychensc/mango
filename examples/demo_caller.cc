#include <memory>
#include <future>
#include <spdlog/spdlog.h>
#include <thread>
#include "caller_builder.h"
#include "director.h"
#include "message_creator.h"
#include "demo_message.h"

using namespace mango;

int main(int argc, char *argv[], char *envp[])
{
    spdlog::set_level(spdlog::level::debug);

    MessageCreator::registerMessageType(PING, createPingMessage);
    MessageCreator::registerMessageType(PONG, createPongMessage);

    std::shared_ptr<Builder> builder = std::make_shared<CallerBuilder>("127.0.0.1", 2024);
    Director director;
    director.setBuilder(builder);

    director.construct();

    auto p_caller = std::dynamic_pointer_cast<Caller>(builder->getResult());

    std::future fut = std::async(std::launch::async, []
                                 { loquat::Epoll::GetInstance()->Wait(); });

    spdlog::debug("start caller");

    p_caller->Connect("127.0.0.1", 2012);

    PingMessage ping;
    p_caller->cast(ping);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto reply = p_caller->call(ping);
    if (reply)
    {
        Context ctx;
        reply->OnCall(ctx);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    spdlog::debug("caller stop");

    loquat::Epoll::GetInstance()->Terminate();
    fut.get();
    return 0;
}