#pragma once

#include "uv.h"
#include "UvException.h"
#include "SocketOperation.h"
#include <vcclr.h>
#include <msclr\marshal.h>

using namespace msclr::interop;
using namespace System::Net;
using namespace System::Runtime::InteropServices;

namespace SocketEventLoop
{
    ref class Socket;

    typedef void (__stdcall* SocketCallback)(Socket^, Exception^);

    public delegate void SocketHandler(Socket^);
    public delegate void ErrorHandler(Socket^, Exception^);

    public ref class Socket sealed
    {
    public:
        void Connect(IPAddress^ ipAddress, int port);
        void Send();
        void Receive();
        void Close();
        void SetBuffer(array<byte>^ buffer, int offset, int count);
        void SetBuffer(int offset, int count);
        ~Socket();

        SocketHandler^ OnConnect;
        SocketHandler^ OnSend;
        SocketHandler^ OnReceive;
        SocketHandler^ OnClose;

        ErrorHandler^ OnError;

        property array<byte>^ Buffer
        {
            array<byte>^ get();
        }

        property int Offset
        {
            int get();
        }

        property int Count
        {
            int get();
        }

        property int BytesReceived
        {
            int get();
            internal: void set(int value);
        }

        property Object^ Data
        { 
            Object^ get();
            void set(Object^ value);
        }

        property SocketOperation CurrentOperation
        { 
            SocketOperation get();
        }

        property int Timeout
        {
            int get();
        }

    internal:
        Socket(uv_loop_t* loop, SocketCallback callback, int timeout);

        property SocketOperation LastOperation
        { 
            SocketOperation get();
            void set(SocketOperation value);
        }

    private:
        uv_loop_t* _loop;
        uv_tcp_t* _tcp;
        uv_timer_t* _timer;

        Object^ _data;
        SocketCallback _callback;
        SocketOperation _lastOperation;
        SocketOperation _currentOperation;
        GCHandle _bufferGCHandle;
        array<byte>^ _buffer;
        int _offset;
        int _count;
        int _bytesReceived;
        int _timeout;
    };
}

using namespace System;
using namespace SocketEventLoop;

struct SocketData
{
    gcroot<Socket^> socketGCHandle;
    SocketCallback callback;
    uv_timer_t* timer;
};

void ConnectCallback(uv_connect_t* connectRequest, int status);
void WriteCallback(uv_write_t* writeRequest, int status);
void ReadCallback(uv_stream_t* client, ssize_t nread, uv_buf_t buffer);
void CloseCallback(uv_handle_t* handle);
void TimeoutCallback(uv_timer_t* timer, int status);
uv_buf_t AllocCallback(uv_handle_t* handle, size_t size);