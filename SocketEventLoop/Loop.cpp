#include "Loop.h"

namespace SocketEventLoop
{
    private delegate void SocketCallbackDelegate(Socket^, Exception^);

    static void SocketCallbackHandler(Socket^ socket, Exception^ exception)
    {
        if (exception != nullptr)
        {
            if (socket->OnError != nullptr)
            {
                socket->OnError(socket, exception);
            }
            return;
        }

        switch (socket->LastOperation)
        {
            case SocketOperation::Connect:
                if (socket->OnConnect != nullptr)
                {
                    socket->OnConnect(socket);
                }
                break;
            case SocketOperation::Send:
                if (socket->OnSend != nullptr)
                {
                    socket->OnSend(socket);
                }
                break;
            case SocketOperation::Receive:
                if (socket->OnReceive != nullptr)
                {
                    socket->OnReceive(socket);
                }
                break;
            case SocketOperation::Close:
                if (socket->OnClose != nullptr)
                {
                    socket->OnClose(socket);
                }
                break;
            default:
                break;
        }
    }

    Loop::Loop()
    {
        SocketCallbackDelegate^ socketCallback = gcnew SocketCallbackDelegate(&SocketCallbackHandler);
        IntPtr socketCallbackPtr = Marshal::GetFunctionPointerForDelegate(socketCallback);
        _socketCallback = static_cast<SocketCallback>(socketCallbackPtr.ToPointer());
        _socketCallbackHandle = GCHandle::Alloc(socketCallback);
        _loop = uv_loop_new();
    }

    void Loop::Run()
    {
        uv_run(_loop, UV_RUN_DEFAULT);
    }

    Socket^ Loop::CreateSocket()
    {
        return CreateSocket(DefaultTimeout);
    }

    Socket^ Loop::CreateSocket(int timeout)
    {
        return gcnew Socket(_loop, _socketCallback, timeout);
    }

    Loop::~Loop()
    {
        if (_loop != NULL)
        {
            uv_loop_delete(_loop);
            _loop = NULL;
        }

        if (_socketCallbackHandle.IsAllocated)
        {
            _socketCallbackHandle.Free();
        }
    }
}
