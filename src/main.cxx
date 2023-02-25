#include<asio.hpp>
#include <chrono>
#include <cstdio>
#include <thread>


#include <unifex/delay.hpp>
#include <unifex/for_each.hpp>
#include <unifex/inplace_stop_token.hpp>
#include <unifex/range_stream.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/typed_via_stream.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/then.hpp>
#include <unifex/stop_when.hpp>
#include <unifex/stop_immediately.hpp>
#include <unifex/take_until.hpp>
#include <unifex/task.hpp>
#include <unifex/single.hpp>
#include <unifex/let_done.hpp>
#include <unifex/then.hpp>
#include <unifex/just.hpp>
#include <unifex/done_as_optional.hpp>

using namespace unifex;
using namespace std::chrono;



#include "asio_coro_util.hpp"


int test_unifex() {

  timed_single_thread_context context;

  auto startTime = steady_clock::now();

  sync_wait(
      stop_when(
          for_each(
              delay(range_stream{0, 100}, context.get_scheduler(), 100ms),
              [startTime](int value) {
                auto ms = duration_cast<milliseconds>(steady_clock::now() - startTime);
                std::printf("[%i ms] %i\n", (int)ms.count(), value);
              }),
          then(
            schedule_after(context.get_scheduler(), 500ms),
            [] { std::printf("cancelling\n"); })));

  return 0;
}


template<typename Sender>
auto done_as_void(Sender&& sender) {
  return let_done((Sender&&)sender, [] { return just(); });
}


int test_unifex_coroutine() {
  timed_single_thread_context context;

  auto makeTask = [&]() -> task<int> {
    auto startTime = steady_clock::now();

    auto s = take_until(
        stop_immediately<int>(
            delay(range_stream{0, 100}, context.get_scheduler(), 50ms)),
        single(schedule_after(context.get_scheduler(), 500ms)));

    int sum = 0;
    while (auto value = co_await done_as_optional(next(s))) {
      auto ms = duration_cast<milliseconds>(steady_clock::now() - startTime);
      std::printf("[%i ms] %i\n", (int)ms.count(), *value);
      std::fflush(stdout);

      sum += *value;
    }

    co_await done_as_void(cleanup(s));

    auto ms = duration_cast<milliseconds>(steady_clock::now() - startTime);
    std::printf("[%i ms] sum = %i\n", (int)ms.count(), sum);
    std::fflush(stdout);

    co_return sum;
  };

  sync_wait(makeTask());

  return 0;
}


#include <async_simple/Try.h>
#include <async_simple/coro/Sleep.h>
#include <async_simple/executors/SimpleExecutor.h>
using namespace async_simple::coro;
using namespace async_simple;

std::atomic<int> check{0};
class SimpleExecutorTest : public executors::SimpleExecutor {
public:
    SimpleExecutorTest(size_t tn) : SimpleExecutor(tn) {}
    Context checkout() override {
        check++;
        return SimpleExecutor::checkout();
    }
    bool checkin(Func func, Context ctx, ScheduleOptions opts) override {
        // -1 is invalid ctx for SimpleExecutor
        if (ctx == (void*)-1) {
            return false;
        }
        check--;
        return SimpleExecutor::checkin(func, ctx, opts);
    }
};

  // Return 43 after 10s.
  Lazy<int> foo() {
     co_await sleep(2s);
     co_return 43;
 }

// To get the value wrapped in Lazy, we could co_await it like:

  Lazy<int> bar() {
      // This would return the value foo returned.
      co_return co_await foo();
 }

// If we don't want the caller to be a coroutine too, we could use Lazy::start
// to get the value asynchronously.
 void foo_use() {
     SimpleExecutorTest e1(10);
     foo().via(&e1).start([](async_simple::Try<int> &&value){
         std::cout << "foo: " << value.value() << "\n";
     });
 }

// When the foo gets its value, the value would be passed to the lambda in
// Lazy::start().
//
// If the user wants to get the value synchronously, he could use
// async_simple::coro::syncAwait.
 void foo_use2() {
     SimpleExecutorTest e1(10);
     auto val = async_simple::coro::syncAwait(foo().via(&e1));
     std::cout << "foo: " << val << "\n";
 }



#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
#include <cstdio>

using asio::ip::tcp;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

#if defined(ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

awaitable<void> echo(tcp::socket socket)
{
  try
  {
    char data[1024];
    for (;;)
    {
      std::size_t n = co_await socket.async_read_some(asio::buffer(data), use_awaitable);
      co_await async_write(socket, asio::buffer(data, n), use_awaitable);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

awaitable<void> listener()
{
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, {tcp::v4(), 55555});
  for (;;)
  {
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    co_spawn(executor, echo(std::move(socket)), detached);
  }
}

int test_asio()
{
  try
  {
    asio::io_context io_context(1);

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });

    co_spawn(io_context, listener(), detached);

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
  }
}


Lazy<int> foo2() {
   co_await sleep(2s);
   co_return 43;
}


int main(int argc, char** argv) {
//    test_unifex();
//    test_unifex_coroutine();
    
    foo_use();
//    foo_use2();
    return 0;
}
