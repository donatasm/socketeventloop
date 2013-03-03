#pragma once

#include "uv.h"
#include "Socket.h"
#include "UvException.h"

using namespace System::Runtime::InteropServices;

namespace SocketEventLoop
{
    public ref class Loop sealed
    {
    public:
        Loop();
        void Run();
        Socket^ CreateSocket();
        Socket^ CreateSocket(int timeout);
        ~Loop();

    internal:
        uv_loop_t* _loop;

    private:
        GCHandle _socketCallbackHandle;
        SocketCallback _socketCallback;
        static initonly int DefaultTimeout = 1000; // 1 second
    };
}