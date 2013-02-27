#pragma once

namespace SocketEventLoop
{
    public enum class SocketOperation
    {
        None,
        Connect,
        Send,
        Receive,
        Close
    };
}