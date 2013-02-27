#include "UvException.h"

namespace SocketEventLoop
{
    UvException::UvException(String^ message) : Exception(message)
    {
    }

    UvException^ UvException::CreateFromLastError(uv_loop_t* loop)
    {
        const char* error = uv_err_name(uv_last_error(loop));
        return gcnew UvException(gcnew String(error));
    }

    void UvException::Throw(uv_loop_t* loop)
    {
        throw CreateFromLastError(loop);
    }
}
