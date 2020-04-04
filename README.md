<p align="center"> 
  <img src="https://i.imgur.com/EqHM4Yz.png" alt="alt logo">
</p>

[![GitHub release](https://img.shields.io/github/release/nxrighthere/NanoSockets.svg)](https://github.com/nxrighthere/NanoSockets/releases) [![PayPal](https://github.com/Rageware/Shields/blob/master/paypal.svg)](https://www.paypal.me/nxrighthere) [![Bountysource](https://github.com/Rageware/Shields/blob/master/bountysource.svg)](https://salt.bountysource.com/checkout/amount?team=nxrighthere) [![Coinbase](https://github.com/Rageware/Shields/blob/master/coinbase.svg)](https://commerce.coinbase.com/checkout/03e11816-b6fc-4e14-b974-29a1d0886697)

This is a highly portable, lightweight and straightforward, zero-cost abstraction of UDP sockets with dual-stack IPv4/IPv6 support for rapid implementation of message-oriented protocols. The library is designed for cross-language compatibility with C, C++, C#, and other languages. For .NET environment, functions support blittable pointers as an alternative to managed types for usage with unmanaged memory allocator.

Building
--------
The native library can be built using [CMake](https://cmake.org/download/) with GNU Make or Visual Studio.

A managed assembly can be built using any available compiling platform that supports C# 3.0 or higher.

Compiled libraries
--------
You can grab compiled libraries from the [release](https://github.com/nxrighthere/NanoSockets/releases) section.

Binaries are provided only for traditional platforms: Windows, Linux, and macOS (x64).

Supported OS versions:
- Windows 7 or higher
- Linux 4.4 or higher
- macOS 10.12 or higher

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

### Unity
Usage is almost the same as in the .NET environment, except that the console functions must be replaced with functions provided by Unity. If the `UDP.Poll()` will be called in a game loop, then make sure that the timeout parameter set to 0 which means non-blocking. Also, keep Unity run in background by enabling the appropriate option in the player settings.

API reference
--------
### Enumerations
#### Status
Definitions of status types for functions:

`OK`

`Error`

### Structures
#### Socket
Contains a blittable structure with socket handle.

`Socket.handle` a socket handle.

`Socket.IsCreated` checks if a socket is created.

#### Address
Contains a blittable structure with anonymous host data and port number.

`Address.port` a port number.

### Classes
#### UDP
`UDP.Initialize()` initializes the native library. Should be called before starting the work. Returns status with a result.

`UDP.Deinitialize()` deinitializes the native library. Should be called after the work is done.

`UDP.Create(int sendBufferSize, int receiveBufferSize)` creates a new socket with a specified size of buffers for sending and receiving. Returns the `Socket` structure with the handle.

`UDP.Destroy(ref Socket socket)` destroys a socket and reset the handle.

`UDP.Bind(Socket socket, ref Address address)` assigns an address to a socket. The address parameter can be set to `IntPtr.Zero` to let the operating system assign any address. Returns 0 on success or != 0 on failure.

`UDP.Connect(Socket socket, ref Address address)` connects a socket to an address. Returns 0 on success or != 0 on failure. 

`UDP.SetOption(Socket socket, int level, int optionName, ref int optionValue, int optionLength)` sets the current value for a socket option associated with a socket. This function can be used to set platform-specific options that were not specified at socket creation by default. Returns status with a result.

`UDP.GetOption(Socket socket, int level, int optionName, ref int optionValue, ref int optionLength)` gets the current value for a socket option associated with a socket. A length of an option value should be initially set to an appropriate size. Returns status with a result.

`UDP.SetNonBlocking(Socket socket)` sets a non-blocking I/O mode for a socket. Returns status with a result.

`UDP.Poll(Socket socket, long timeout)` determines the status of a socket and waiting if necessary. This function can be used for readiness-oriented receiving. The timeout parameter may be specified in milliseconds to control polling duration. If a timeout of 0 is specified, this function will return immediately. If the time limit expired it will return 0. If a socket is ready for receiving it will return 1. Otherwise, it will return < 0 if an error occurred.

`UDP.Send(Socket socket, ref Address address, byte[] buffer, int bufferLength)` sends a message to the specified address of a receiver. The address parameter can be set to `IntPtr.Zero` if a socket is connected to an address. A pointer `IntPtr` to a native buffer can be used instead of a reference to a byte array. Returns the total number of bytes sent, which can be less than the number indicated by the buffer length. Otherwise, it will return < 0 if an error occurred.

`UDP.Receive(Socket socket, ref Address address, byte[] buffer, int bufferLength)` receives a message and obtains the address of a sender. The address parameter can be set to `IntPtr.Zero` to skip the address obtainment. A pointer `IntPtr` to a native buffer can be used instead of a reference to a byte array. Returns the total number of bytes received. Otherwise, it will return < 0 if an error occurred.

`UDP.GetAddress(Socket socket, ref Address address)` gets an address from a bound or connected socket. This function is especially useful to determine the local association that has been set by the operating system. Returns status with a result.

`UDP.IsEqual(ref Address left, ref Address right)` compares two addresses for equality. Returns status with a result.

`UDP.SetIP(ref Address address, string ip)` sets an IP address. A pointer `IntPtr` can be used instead of the immutable string. Returns status with a result.

`UDP.GetIP(ref Address address, StringBuilder ip, int ipLength)` gets an IP address. The capacity of the mutable string should be equal to `UDP.hostNameSize` constant field. A pointer `IntPtr` can be used instead of the mutable string. Returns status with a result.

`UDP.SetHostName(ref Address address, string name)` sets host name or an IP address. A pointer `IntPtr` can be used instead of the immutable string. Returns status with a result.

`UDP.GetHostName(ref Address address, StringBuilder name, int nameLength)` attempts to do a reverse lookup from the address. A pointer `IntPtr` can be used instead of the mutable string. Returns status with a result.
