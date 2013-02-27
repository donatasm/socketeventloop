using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using NUnit.Framework;

namespace SocketEventLoop.Tests
{
    [TestFixture]
    internal class TimeoutTest
    {
        [Test]
        public void Connect_Timeouts_After_1000ms_Server_Not_Available()
        {
            using (var loop = new Loop())
            {
                var errors = new List<Exception>();
                double elapsedMs = 0;
                var operation = SocketOperation.None;

                using (var socket = loop.CreateSocket(1000))
                {
                    socket.OnError += (client, error) =>
                    {
                        errors.Add(error);

                        if (error is TimeoutException)
                        {
                            var sw = (Stopwatch) client.Data;
                            elapsedMs = sw.ElapsedMilliseconds;
                            operation = client.CurrentOperation;
                        }
                    };

                    socket.Data = Stopwatch.StartNew();
                    socket.Connect(new IPAddress(new byte[] {1, 2, 3, 4}), 1); // try to connect to unreachable address
                    loop.Run();

                    Assert.AreEqual(2, errors.Count);
                    Assert.IsInstanceOf<TimeoutException>(errors[0]);
                    StringAssert.IsMatch("Connect timeout: (.*) ms", errors[0].Message);
                    Assert.IsInstanceOf<UvException>(errors[1]);
                    Assert.AreEqual("ETIMEDOUT", errors[1].Message);
                    Assert.AreEqual(SocketOperation.Connect, operation);
                    Assert.AreEqual(1000d, elapsedMs, 50);
                }
            }
        }
    }
}
