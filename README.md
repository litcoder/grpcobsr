# Event subscription with gRPC
Event subscription example on gRPC.

## Build
```
mkdir -p build && cd $_
cmake ..
make -j $(nproc)
```

## Run
From a terminal application, run `./server` and `./client` respectively. Note that server accepts number of events to be sent to client, 0 will be set if not given which indicates infinite events.

![running](./doc/grpc_observer.png)