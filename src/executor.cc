#include "executor.h"
#include "loquat/include/epoll.h"

namespace mango
{
    void Executor::start()
    {
        fut_ = std::async(std::launch::async, []
                          { loquat::Epoll::GetInstance().Wait(); });
    }

    void Executor::stop()
    {
        loquat::Epoll::GetInstance().Terminate();
        fut_.wait();
    }
}
