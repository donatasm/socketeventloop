#include "Socket.h"

#define FREE_GC_HANDLE(gcHandle) if (gcHandle.IsAllocated) gcHandle.Free();

namespace SocketEventLoop
{
    Socket::Socket(uv_loop_t* loop, SocketCallback callback, int timeout)
    {
        _loop = loop;
        _callback = callback;
        _buffer = nullptr;
        _bytesReceived = 0;
        _count = 0;
        _offset = 0;
        _data = nullptr;

        _timeout = timeout;

        _currentOperation = SocketOperation::None;
        _lastOperation = SocketOperation::None;

        // create and init socket
        _tcp = new uv_tcp_t;
        if (uv_tcp_init(_loop, _tcp))
        {
            UvException::Throw(_loop);
        }

        // create timer for timeout tracking
        _timer = new uv_timer_t;
        if (uv_timer_init(_loop, _timer))
        {
            UvException::Throw(_loop);
        }
    }

    void Socket::Connect(IPAddress^ ipAddress, int port)
    {
        if (ipAddress == nullptr) throw gcnew ArgumentNullException("ipAddress");
        if (port < 1) throw gcnew ArgumentOutOfRangeException("port");

        _currentOperation = SocketOperation::Connect;

        // init socket data
        SocketData* data = new SocketData;
        data->socketGCHandle = this;
        data->callback = _callback;
        data->timer = _timer;

        _tcp->data = data;
        _timer->data = data;
        
        // create destination address sruct
        marshal_context context;
        const char* addressPtr = context.marshal_as<const char*>(ipAddress->ToString());
        struct sockaddr_in address = uv_ip4_addr(addressPtr, port);

        // create a connect request
        uv_connect_t* connectRequest = new uv_connect_t;
        if (uv_tcp_connect(connectRequest, _tcp, address, ConnectCallback))
        {
            delete data;
            delete connectRequest;
            UvException::Throw(_loop);
        }

        // start timer
        uv_timer_start(_timer, TimeoutCallback, _timeout, 0);
    }

    void Socket::Send()
    {
        _currentOperation = SocketOperation::Send;

        IntPtr bufferStartPtr = IntPtr::Zero;

        if (_buffer != nullptr)
        {
            bufferStartPtr = Marshal::UnsafeAddrOfPinnedArrayElement(_buffer, _offset);
        }

        char* bufferStart = reinterpret_cast<char*>(bufferStartPtr.ToPointer());

        uv_buf_t buffer = uv_buf_init(bufferStart, _count);
        uv_write_t* writeRequest = new uv_write_t;

        if (uv_write(writeRequest, (uv_stream_t*)_tcp, &buffer, 1, WriteCallback))
        {
            delete writeRequest;
            UvException::Throw(_tcp->loop);
        }
    }

    void Socket::Receive()
    {
        _currentOperation = SocketOperation::Receive;

        // TODO: handle when buffer is not set, or it's length is 0

        if (uv_read_start((uv_stream_t*)_tcp, AllocCallback, ReadCallback))
        {
            UvException::Throw(_tcp->loop);
        }
    }

    void Socket::Close()
    {
        _currentOperation = SocketOperation::Close;

        uv_close((uv_handle_t*)_tcp, CloseCallback);
    }

    void Socket::SetBuffer(array<byte>^ buffer, int offset, int count)
    {
        if (buffer == nullptr)
            throw gcnew ArgumentNullException("buffer", "Buffer cannot be set to null.");

        if ((count < 0) || (count > buffer->Length))
            throw gcnew ArgumentOutOfRangeException("count", "Count cannot be negative or greater than buffer length.");

        if ((offset < 0) || (offset > count))
            throw gcnew ArgumentOutOfRangeException("offset", "Offset cannot be negative or greater than count.");

        if (!Object::ReferenceEquals(buffer, _buffer))
        {
            FREE_GC_HANDLE(_bufferGCHandle);
            _bufferGCHandle = GCHandle::Alloc(_buffer, Runtime::InteropServices::GCHandleType::Pinned);
        }

        _buffer = buffer;
        _offset = offset;
        _count = count;
    }

    void Socket::SetBuffer(int offset, int count)
    {
        SetBuffer(_buffer, offset, count);
    }

    Socket::~Socket()
    {
        FREE_GC_HANDLE(_bufferGCHandle);

        SocketData* data = (SocketData*)_tcp->data;

        delete data;
        delete _tcp;
        delete _timer;
    }

    SocketOperation Socket::LastOperation::get()
    {
        return _lastOperation;
    }

    void Socket::LastOperation::set(SocketOperation value)
    {
        _lastOperation = value;
    }

    array<byte>^ Socket::Buffer::get()
    {
        return _buffer;
    }

    int Socket::Offset::get()
    {
        return _offset;
    }

    int Socket::Count::get()
    {
        return _count;
    }

    int Socket::BytesReceived::get()
    {
        return _bytesReceived;
    }

    void Socket::BytesReceived::set(int value)
    {
        _bytesReceived = value;
    }

    Object^ Socket::Data::get()
    {
        return _data;
    }

    void Socket::Data::set(Object^ value)
    {
        _data = value;
    }

    SocketOperation Socket::CurrentOperation::get()
    {
        return _currentOperation;
    }

    int Socket::Timeout::get()
    {
        return _timeout;
    }
}

// unmanaged

void ConnectCallback(uv_connect_t* connectRequest, int status)
{
    uv_tcp_t* tcp = (uv_tcp_t*)connectRequest->handle;
    delete connectRequest;

    SocketData* data = (SocketData*)tcp->data;
    SocketCallback connectCallback = data->callback;
    Socket^ socket = data->socketGCHandle;
    socket->LastOperation = SocketOperation::Connect;
    uv_timer_stop(data->timer);

    if (status)
    {
        UvException^ error = UvException::CreateFromLastError(tcp->loop);
        connectCallback(socket, error);
    }
    else
    {
        connectCallback(socket, nullptr);
    }
}

void WriteCallback(uv_write_t* writeRequest, int status)
{
    uv_tcp_t* tcp = (uv_tcp_t*)writeRequest->handle;
    delete writeRequest;

    SocketData* data = (SocketData*)tcp->data;
    SocketCallback writeCallback = data->callback;
    Socket^ socket = data->socketGCHandle;
    socket->LastOperation = SocketOperation::Send;

    if (status)
    {
        UvException^ error = UvException::CreateFromLastError(tcp->loop);
        writeCallback(socket, error);
    }
    else
    {
        writeCallback(socket, nullptr);
    }
}

void ReadCallback(uv_stream_t* tcp, ssize_t nread, uv_buf_t buffer)
{
    SocketData* data = (SocketData*)tcp->data;
    SocketCallback readCallback = data->callback;
    Socket^ socket = data->socketGCHandle;
    socket->LastOperation = SocketOperation::Receive;

    socket->BytesReceived = (int)nread;
    readCallback(socket, nullptr);
}

void CloseCallback(uv_handle_t* handle)
{
    uv_tcp_t* tcp = (uv_tcp_t*)handle;

    SocketData* data = (SocketData*)tcp->data;
    SocketCallback closeCallback = data->callback;
    Socket^ socket = data->socketGCHandle;
    socket->LastOperation = SocketOperation::Close;

    closeCallback(socket, nullptr);
}

void TimeoutCallback(uv_timer_t* timer, int status)
{
    SocketData* data = (SocketData*)timer->data;
    SocketCallback timeoutCallback = data->callback;
    Socket^ socket = data->socketGCHandle;

    SocketOperation operation = socket->CurrentOperation;
    String^ message = String::Format("{0} timeout: {1} ms", operation, socket->Timeout);
    TimeoutException^ timeout = gcnew TimeoutException(message);

    timeoutCallback(socket, timeout);
}

uv_buf_t AllocCallback(uv_handle_t* handle, size_t size)
{
    uv_tcp_t* tcp = (uv_tcp_t*)handle;

    SocketData* data = (SocketData*)tcp->data;
    Socket^ socket = data->socketGCHandle;

    IntPtr bufferStartPtr = IntPtr::Zero;

    if (socket->Buffer != nullptr)
    {
        bufferStartPtr = Marshal::UnsafeAddrOfPinnedArrayElement(socket->Buffer, socket->Offset);
    }

    char* bufferStart = reinterpret_cast<char*>(bufferStartPtr.ToPointer());

    return uv_buf_init(bufferStart, socket->Count);
}