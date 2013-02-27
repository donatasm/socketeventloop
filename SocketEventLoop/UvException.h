#pragma once

#include "uv.h"

using namespace System;

namespace SocketEventLoop
{
    public ref class UvException sealed : Exception
    {
    internal:
        UvException(String^ message);
        static UvException^ CreateFromLastError(uv_loop_t* loop);
        static void Throw(uv_loop_t* loop);
    };
}

