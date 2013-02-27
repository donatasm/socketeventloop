using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace SocketEventLoop.Tests
{
    [TestFixture]
    internal class SendReceiveTest
    {
        private const byte EndChar = (byte)'$';

        [Test]
        public void Send_Receive_Echo_Server_Events()
        {
            var sendData = new byte[] { 1, 2, 3, 4, 5, EndChar };
            var receiveData = new byte[sendData.Length - 1];
            var callbacks = new List<Callback>();

            using (var loop = new Loop())
            {
                using (var socket = loop.CreateSocket())
                {
                    socket.OnError += AssertFail;
                    socket.OnConnect += client =>
                    {
                        callbacks.Add(Callback.Connect);
                        client.SetBuffer(sendData, 0, sendData.Length);
                        client.Send();
                    };
                    socket.OnSend += client =>
                    {
                        callbacks.Add(Callback.Send);
                        client.SetBuffer(receiveData, 0, receiveData.Length);
                        client.Receive();
                    };
                    socket.OnReceive += client =>
                    {
                        callbacks.Add(Callback.Receive);
                        client.Close();
                    };
                    socket.OnClose += client => callbacks.Add(Callback.Close);
                    socket.Connect(Server.IpAddress, Server.Port);

                    loop.Run();
                }
            }

            CollectionAssert.AreEqual(sendData.Take(sendData.Length - 1), receiveData);
            CollectionAssert.AreEqual(new[] { Callback.Connect, Callback.Send, Callback.Receive, Callback.Close }, callbacks);
        }

        private static readonly ErrorHandler AssertFail = (socket, error) => Assert.Fail(error.Message);

        private enum Callback : byte
        {
            Connect, Send, Receive, Close
        }
    }
}
