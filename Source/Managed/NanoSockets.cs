/****************************** Module Header ******************************\
* Module Name:  Sockets.cs
* Project:      Sockets
* Copyright (c) 2019 Stanislav Denisov
*
* Lightweight UDP sockets abstraction for rapid implementation of message-oriented protocols
*
* This source is subject to the Microsoft Public License.
* See https://opensource.org/licenses/MS-PL
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

using System;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

namespace NanoSockets {
	public enum Status {
		OK = 0,
		Error = -1
	}

	public struct Socket {
		public long handle;

		public bool IsCreated {
			get {
				return handle > 0;
			}
		}
	}

	[StructLayout(LayoutKind.Explicit, Size = 20)]
	public struct Address {
        [FieldOffset(16)]
		public ushort port;
	}

	[SuppressUnmanagedCodeSecurity]
	public static class UDP {
		#if __IOS__ || UNITY_IOS && !UNITY_EDITOR
			private const string nativeLibrary = "__Internal";
		#else
			private const string nativeLibrary = "nanosockets";
		#endif

        public const int hostNameSize = 1025;

		[DllImport(nativeLibrary, EntryPoint = "nanosockets_initialize", CallingConvention = CallingConvention.Cdecl)]
		public static extern Status Initialize();

		[DllImport(nativeLibrary, EntryPoint = "nanosockets_detiniailize", CallingConvention = CallingConvention.Cdecl)]
		public static extern void Deinitialize();

		[DllImport(nativeLibrary, EntryPoint = "nanosockets_create", CallingConvention = CallingConvention.Cdecl)]
		public static extern Socket Create(int sendBufferSize, int receiveBufferSize);

		[DllImport(nativeLibrary, EntryPoint = "nanosockets_destroy", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Destroy(ref Socket socket);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_bind", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Bind(Socket socket, ref Address address);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_connect", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Connect(Socket socket, ref Address address);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_set_nonblocking", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status SetNonBlocking(Socket socket);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_poll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Poll(Socket socket, long milliseconds);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_send", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Send(Socket socket, IntPtr address, IntPtr buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_send", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Send(Socket socket, IntPtr address, byte[] buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_send", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Send(Socket socket, ref Address address, IntPtr buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_send", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Send(Socket socket, ref Address address, byte[] buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_receive", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Receive(Socket socket, IntPtr address, IntPtr buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_receive", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Receive(Socket socket, IntPtr address, byte[] buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_receive", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Receive(Socket socket, ref Address address, IntPtr buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_receive", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Receive(Socket socket, ref Address address, byte[] buffer, int bufferLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_is_equal", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status IsEqual(ref Address left, ref Address right);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_set_ip", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status SetIP(ref Address address, IntPtr ip);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_set_ip", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status SetIP(ref Address address, string ip);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_get_ip", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status GetIP(ref Address address, IntPtr ip, int ipLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_get_ip", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status GetIP(ref Address address, StringBuilder ip, int ipLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_set_hostname", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status SetHostName(ref Address address, IntPtr name);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_set_hostname", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status SetHostName(ref Address address, string name);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_get_hostname", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status GetHostName(ref Address address, IntPtr name, int nameLength);

        [DllImport(nativeLibrary, EntryPoint = "nanosockets_address_get_hostname", CallingConvention = CallingConvention.Cdecl)]
        public static extern Status GetHostName(ref Address address, StringBuilder name, int nameLength);
    }
}