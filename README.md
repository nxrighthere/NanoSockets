<p align="center"> 
  <img src="https://i.imgur.com/EqHM4Yz.png" alt="alt logo">
</p>

[![GitHub release](https://img.shields.io/github/release/nxrighthere/NanoSockets.svg)](https://github.com/nxrighthere/NanoSockets/releases) [![NuGet](https://img.shields.io/nuget/v/NanoSockets.svg)](https://www.nuget.org/packages/NanoSockets/) [![PayPal](https://drive.google.com/uc?id=1OQrtNBVJehNVxgPf6T6yX1wIysz1ElLR)](https://www.paypal.me/nxrighthere) [![Bountysource](https://drive.google.com/uc?id=19QRobscL8Ir2RL489IbVjcw3fULfWS_Q)](https://salt.bountysource.com/checkout/amount?team=nxrighthere) [![Coinbase](https://drive.google.com/uc?id=1LckuF-IAod6xmO9yF-jhTjq1m-4f7cgF)](https://commerce.coinbase.com/checkout/03e11816-b6fc-4e14-b974-29a1d0886697)

This is a highly portable, lightweight and straightforward, zero-cost abstraction of UDP sockets with dual-stack IPv4/IPv6 support for rapid implementation of message-oriented protocols. The library is designed for cross-language compatibility with C, C++, C# and other languages.

Building
--------
The native library can be built using [CMake](https://cmake.org/download/) with GCC or Clang.

A managed assembly can be built using any available compiling platform that supports C# 3.0 or higher.

Compiled libraries
--------
You can grab compiled libraries from the [release](https://github.com/nxrighthere/NanoSockets/releases) section or from [NuGet](https://www.nuget.org/packages/NanoSockets).

Binaries are provided only for traditional platforms: Windows, Linux, and macOS (x64).

Usage
--------
Before starting to work, the library should be initialized using `NanoSockets.UDP.Initialize();` function.

After the work is done, deinitialize the library using `NanoSockets.UDP.Deinitialize();` function.

### .NET environment
##### Start a new server:
```c#
Socket server = UDP.Create(256 * 1024, 256 * 1024);
Address listenAddress = new Address();

listenAddress.port = port;

if (UDP.SetIP(ref listenAddress, "::0") == Status.OK)
    Console.WriteLine("Address set!");

if (UDP.Bind(server, ref listenAddress) == 0)
    Console.WriteLine("Socket bound!");

UDP.SetNonBlocking(server);

StringBuilder ip = new StringBuilder(UDP.hostNameSize);
Address address = new Address();
byte[] buffer = new byte[1024];

while (!Console.KeyAvailable) {
    if (UDP.Poll(server, 15) > 0) {
        int dataLength = 0;

        while ((dataLength = UDP.Receive(server, ref address, buffer, buffer.Length)) > 0) {
            UDP.GetIP(ref address, ip, ip.Capacity);

            Console.WriteLine("Message received from - IP: " + ip + ", Data length: " + dataLength);

            UDP.Send(server, ref address, buffer, buffer.Length);
        }
    }
}

UDP.Destroy(ref server);
```

##### Start a new client:
```c#
Socket client = UDP.Create(256 * 1024, 256 * 1024);
Address connectionAddress = new Address();

connectionAddress.port = port;

if (UDP.SetIP(ref connectionAddress, "::1") == Status.OK)
    Console.WriteLine("Address set!");

if (UDP.Connect(client, ref connectionAddress) == 0)
    Console.WriteLine("Socket connected!");

UDP.SetNonBlocking(client);

byte[] buffer = new byte[1024];

UDP.Send(client, IntPtr.Zero, buffer, buffer.Length);

while (!Console.KeyAvailable) {
    if (UDP.Poll(client, 15) > 0) {
        int dataLength = 0;

        while ((dataLength = UDP.Receive(client, IntPtr.Zero, buffer, buffer.Length)) > 0) {
            Console.WriteLine("Message received from server - Data length: " + dataLength);

            UDP.Send(client, IntPtr.Zero, buffer, buffer.Length);
        }
    }
}

UDP.Destroy(ref client);
```

API reference
--------
