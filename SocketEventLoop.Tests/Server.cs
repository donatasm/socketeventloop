using System.Net;

namespace SocketEventLoop.Tests
{
    internal static class Server
    {
        public static readonly IPAddress IpAddress = new IPAddress(new byte[] { 127, 0, 0, 1 });
        public const int Port = 6969;
    }
}
