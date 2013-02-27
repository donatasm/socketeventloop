using System;
using System.Linq;
using NUnit.Framework;

namespace SocketEventLoop.Tests
{
    [TestFixture]
    internal class SetBufferTest
    {
        [Test]
        public void SetBuffer_Sets_Buffer()
        {
            var buffer = Enumerable.Range(1, 100).Select(b => (byte)b).ToArray();

            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket())
                {
                    socket.OnError += AssertFail;
                    socket.OnConnect += client =>
                    {
                        client.SetBuffer(buffer, 1, buffer.Length);
                        CollectionAssert.AreEqual(buffer, client.Buffer);
                        Assert.AreEqual(1, client.Offset);
                        Assert.AreEqual(buffer.Length, client.Count);
                    };
                    socket.Connect(Server.IpAddress, Server.Port);

                    loop.Run();
                }
            }
        }

        [Test]
        public void SetBuffer_Count_Is_Greater_Than_Buffer_Length_Throws_Argument_Out_Of_Range_Exception()
        {
            var buffer = Enumerable.Range(1, 100).Select(b => (byte)b).ToArray();

            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket())
                {
                    socket.OnError += AssertFail;
                    socket.OnConnect += client => Assert.Throws<ArgumentOutOfRangeException>(() => client.SetBuffer(buffer, 0, buffer.Length + 1));
                    socket.Connect(Server.IpAddress, Server.Port);

                    loop.Run();
                }
            }
        }

        [Test]
        public void SetBuffer_Buffer_Is_Null_Throws_Argument_Null_Exception()
        {
            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket())
                {
                    socket.OnError += AssertFail;
                    socket.OnConnect += client => Assert.Throws<ArgumentNullException>(() => client.SetBuffer(null, 0, 0));
                    socket.Connect(Server.IpAddress, Server.Port);

                    loop.Run();
                }
            }
        }

        [Test]
        public void SetBuffer_Offset_Is_Greater_Than_Count()
        {
            var buffer = Enumerable.Range(1, 5).Select(b => (byte)b).ToArray();

            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket())
                {
                    socket.OnError += AssertFail;
                    socket.OnConnect += client => Assert.Throws<ArgumentOutOfRangeException>(() => client.SetBuffer(buffer, 4, 3));
                    socket.Connect(Server.IpAddress, Server.Port);

                    loop.Run();
                }
            }
        }

        [Test]
        public void SetBuffer_Sets_Offset_And_Count()
        {
            var buffer = Enumerable.Range(1, 5).Select(b => (byte)b).ToArray();

            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket())
                {
                    socket.OnError += AssertFail;
                    socket.OnConnect += client =>
                    {
                        client.SetBuffer(buffer, 0, 3);
                        client.SetBuffer(3, 4);
                        Assert.AreEqual(3, client.Offset);
                        Assert.AreEqual(4, client.Count);
                    };
                    socket.Connect(Server.IpAddress, Server.Port);
                    loop.Run();
                }
            }
        }

        private static readonly ErrorHandler AssertFail = (socket, error) => Assert.Fail(error.Message);
    }
}
