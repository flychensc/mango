# mango
Simple RPC framework

## Usage

`Message` is the base class of RPC messages, and all RPC messages inherit from this class to implement their own `OnCall` calls.

`ExecutorService` is an RPC server that usually only needs to `start` a new thread.

`Caller` is an RPC client that provides 2 RPC methods, one is `cast` and one is `call`.

## Examples

Refer [examples](./examples/)

1. RPC server and client:

   [RPC Message](./examples/demo_message.h), [RPC Server](./examples/demo_executor.cc) and [RPC Client](./examples/demo_caller.cc)
