using System;
using System.Diagnostics;
using System.Net;
using NUnit.Framework;

namespace SocketEventLoop.Tests
{
    [TestFixture]
    internal class TimeoutTest
    {
        [Test]
        public void Connect_Timeouts_After_50ms_Server_Not_Available()
        {
            const double timeout = 50;

            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket((int)timeout))
                {
                    socket.OnError += (client, error) =>
                    {
                        if (error is TimeoutException)
                        {
                            var sw = (Stopwatch) client.Data;
                            double elapsedMs = sw.ElapsedMilliseconds;

                            Assert.AreEqual(timeout, elapsedMs, 40); // windows timer is inaccurate
                            Assert.AreEqual(client.CurrentOperation, SocketOperation.Connect);
                            StringAssert.IsMatch("Connect timeout: (.*) ms", error.Message);
                            Assert.Pass("Connect timeout: {0}", elapsedMs); // don't wait for a loop to process the actual timeout 
                        }
                        else
                        {
                            Assert.Fail("Not instance of TimeoutException");                                  
                        }

                    };

                    socket.Data = Stopwatch.StartNew();
                    socket.Connect(new IPAddress(new byte[] {1, 2, 3, 4}), 1); // try to connect to unreachable address
                    loop.Run();
                }
            }
        }
    }
}
