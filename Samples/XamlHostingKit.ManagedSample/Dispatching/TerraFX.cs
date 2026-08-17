#nullable enable

// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from winrt/windows.ui.core.h in the Windows SDK for Windows 10.0.26100.0
// Original source is Copyright © Microsoft. All rights reserved.

using System;
using System.Diagnostics;
using System.Reflection.Metadata;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using Windows.Foundation;
using Windows.UI.Core;
using static TerraFX.Interop.IID;

namespace TerraFX.Interop;

/// <include file='IDispatchedHandler.xml' path='doc/member[@name="IDispatchedHandler"]/*' />
[Guid("D1F276C4-98D8-4636-BF49-EB79507548E9")]
public unsafe partial struct IDispatchedHandler : IDispatchedHandler.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IDispatchedHandler);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IDispatchedHandler*, Guid*, void**, int>)(lpVtbl[0]))((IDispatchedHandler*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IDispatchedHandler*, uint>)(lpVtbl[1]))((IDispatchedHandler*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IDispatchedHandler*, uint>)(lpVtbl[2]))((IDispatchedHandler*)Unsafe.AsPointer(ref this));
    }

    /// <include file='IDispatchedHandler.xml' path='doc/member[@name="IDispatchedHandler.Invoke"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT Invoke()
    {
        return ((delegate* unmanaged[MemberFunction]<IDispatchedHandler*, int>)(lpVtbl[3]))((IDispatchedHandler*)Unsafe.AsPointer(ref this));
    }

    public interface Interface : IUnknown.Interface
    {
        HRESULT Invoke();
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        public delegate* unmanaged[MemberFunction]<TSelf*, Guid*, void**, int> QueryInterface;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> AddRef;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> Release;

        public delegate* unmanaged[MemberFunction]<TSelf*, int> Invoke;
    }
}

public static partial class IID
{
    public static ref readonly Guid IID_IDispatchedHandler
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xC4, 0x76, 0xF2, 0xD1,
                0xD8, 0x98,
                0x36, 0x46,
                0xBF,
                0x49,
                0xEB,
                0x79,
                0x50,
                0x75,
                0x48,
                0xE9
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IUnknown
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0xC0,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x46
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IInspectable
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xE0, 0xE2, 0x86, 0xAF,
                0x2D, 0xB1,
                0x6A, 0x4C,
                0x9C,
                0x5A,
                0xD7,
                0xAA,
                0x65,
                0x10,
                0x1E,
                0x90
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IAsyncAction
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x06, 0x80, 0x64, 0x5A,
                0x3A, 0x84,
                0xA9, 0x4D,
                0x86,
                0x5B,
                0x9D,
                0x26,
                0xE5,
                0xDF,
                0xAD,
                0x7B
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IAsyncActionCompletedHandler
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x81, 0x5C, 0xED, 0xA4,
                0xC9, 0x76,
                0xBD, 0x40,
                0x8B,
                0xE6,
                0xB1,
                0xD9,
                0x0F,
                0xB2,
                0x0A,
                0xE7
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_ICoreDispatcher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xA8, 0x2F, 0xDB, 0x60,
                0x05, 0xB7,
                0xDE, 0x4F,
                0xA7,
                0xD6,
                0xEB,
                0xBB,
                0x18,
                0x91,
                0xD3,
                0x9E
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }
}

public unsafe interface INativeGuid
{
    protected internal static abstract Guid* NativeGuid { get; }
}

public readonly unsafe partial struct HRESULT : IComparable, IComparable<HRESULT>, IEquatable<HRESULT>, IFormattable
{
    public readonly int Value;

    public HRESULT(int value)
    {
        Value = value;
    }

    public static bool operator ==(HRESULT left, HRESULT right) => left.Value == right.Value;

    public static bool operator !=(HRESULT left, HRESULT right) => left.Value != right.Value;

    public static bool operator <(HRESULT left, HRESULT right) => left.Value < right.Value;

    public static bool operator <=(HRESULT left, HRESULT right) => left.Value <= right.Value;

    public static bool operator >(HRESULT left, HRESULT right) => left.Value > right.Value;

    public static bool operator >=(HRESULT left, HRESULT right) => left.Value >= right.Value;

    public static implicit operator HRESULT(byte value) => new HRESULT(value);

    public static explicit operator byte(HRESULT value) => (byte)(value.Value);

    public static implicit operator HRESULT(short value) => new HRESULT(value);

    public static explicit operator short(HRESULT value) => (short)(value.Value);

    public static implicit operator HRESULT(int value) => new HRESULT(value);

    public static implicit operator int(HRESULT value) => value.Value;

    public static explicit operator HRESULT(long value) => new HRESULT(unchecked((int)(value)));

    public static implicit operator long(HRESULT value) => value.Value;

    public static explicit operator HRESULT(nint value) => new HRESULT(unchecked((int)(value)));

    public static implicit operator nint(HRESULT value) => value.Value;

    public static implicit operator HRESULT(sbyte value) => new HRESULT(value);

    public static explicit operator sbyte(HRESULT value) => (sbyte)(value.Value);

    public static implicit operator HRESULT(ushort value) => new HRESULT(value);

    public static explicit operator ushort(HRESULT value) => (ushort)(value.Value);

    public static explicit operator HRESULT(uint value) => new HRESULT(unchecked((int)(value)));

    public static explicit operator uint(HRESULT value) => (uint)(value.Value);

    public static explicit operator HRESULT(ulong value) => new HRESULT(unchecked((int)(value)));

    public static explicit operator ulong(HRESULT value) => (ulong)(value.Value);

    public static explicit operator HRESULT(nuint value) => new HRESULT(unchecked((int)(value)));

    public static explicit operator nuint(HRESULT value) => (nuint)(value.Value);

    public int CompareTo(object? obj)
    {
        if (obj is HRESULT other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException("obj is not an instance of HRESULT.");
    }

    public int CompareTo(HRESULT other) => Value.CompareTo(other.Value);

    public override bool Equals(object? obj) => (obj is HRESULT other) && Equals(other);

    public bool Equals(HRESULT other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value.ToString("X8");

    public string ToString(string? format, IFormatProvider? formatProvider) => Value.ToString(format, formatProvider);
}

[Guid("00000000-0000-0000-C000-000000000046")]
public unsafe partial struct IUnknown : IUnknown.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IUnknown);

    public void** lpVtbl;

    /// <include file='IUnknown.xml' path='doc/member[@name="IUnknown.QueryInterface"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IUnknown*, Guid*, void**, int>)(lpVtbl[0]))((IUnknown*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <include file='IUnknown.xml' path='doc/member[@name="IUnknown.AddRef"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IUnknown*, uint>)(lpVtbl[1]))((IUnknown*)Unsafe.AsPointer(ref this));
    }

    /// <include file='IUnknown.xml' path='doc/member[@name="IUnknown.Release"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IUnknown*, uint>)(lpVtbl[2]))((IUnknown*)Unsafe.AsPointer(ref this));
    }

    public interface Interface : INativeGuid
    {
        HRESULT QueryInterface(Guid* riid, void** ppvObject);

        uint AddRef();

        uint Release();
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        public delegate* unmanaged[MemberFunction]<TSelf*, Guid*, void**, int> QueryInterface;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> AddRef;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> Release;
    }
}

[Guid("5A648006-843A-4DA9-865B-9D26E5DFAD7B")]
public unsafe partial struct IAsyncAction : IAsyncAction.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IAsyncAction);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, Guid*, void**, int>)(lpVtbl[0]))((IAsyncAction*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, uint>)(lpVtbl[1]))((IAsyncAction*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, uint>)(lpVtbl[2]))((IAsyncAction*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IInspectable.GetIids" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetIids(uint* iidCount, Guid** iids)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, uint*, Guid**, int>)(lpVtbl[3]))((IAsyncAction*)Unsafe.AsPointer(ref this), iidCount, iids);
    }

    /// <inheritdoc cref="IInspectable.GetRuntimeClassName" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetRuntimeClassName(HSTRING* className)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, HSTRING*, int>)(lpVtbl[4]))((IAsyncAction*)Unsafe.AsPointer(ref this), className);
    }

    /// <inheritdoc cref="IInspectable.GetTrustLevel" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetTrustLevel(TrustLevel* trustLevel)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, TrustLevel*, int>)(lpVtbl[5]))((IAsyncAction*)Unsafe.AsPointer(ref this), trustLevel);
    }

    /// <include file='IAsyncAction.xml' path='doc/member[@name="IAsyncAction.put_Completed"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT put_Completed(IAsyncActionCompletedHandler* handler)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, IAsyncActionCompletedHandler*, int>)(lpVtbl[6]))((IAsyncAction*)Unsafe.AsPointer(ref this), handler);
    }

    /// <include file='IAsyncAction.xml' path='doc/member[@name="IAsyncAction.get_Completed"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT get_Completed(IAsyncActionCompletedHandler** handler)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, IAsyncActionCompletedHandler**, int>)(lpVtbl[7]))((IAsyncAction*)Unsafe.AsPointer(ref this), handler);
    }

    /// <include file='IAsyncAction.xml' path='doc/member[@name="IAsyncAction.GetResults"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetResults()
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncAction*, int>)(lpVtbl[8]))((IAsyncAction*)Unsafe.AsPointer(ref this));
    }

    public interface Interface : IInspectable.Interface
    {
        HRESULT put_Completed(IAsyncActionCompletedHandler* handler);

        HRESULT get_Completed(IAsyncActionCompletedHandler** handler);

        HRESULT GetResults();
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        public delegate* unmanaged[MemberFunction]<TSelf*, Guid*, void**, int> QueryInterface;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> AddRef;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> Release;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint*, Guid**, int> GetIids;

        public delegate* unmanaged[MemberFunction]<TSelf*, HSTRING*, int> GetRuntimeClassName;

        public delegate* unmanaged[MemberFunction]<TSelf*, TrustLevel*, int> GetTrustLevel;

        public delegate* unmanaged[MemberFunction]<TSelf*, IAsyncActionCompletedHandler*, int> put_Completed;

        public delegate* unmanaged[MemberFunction]<TSelf*, IAsyncActionCompletedHandler**, int> get_Completed;

        public delegate* unmanaged[MemberFunction]<TSelf*, int> GetResults;
    }
}

public readonly unsafe partial struct HSTRING : IComparable, IComparable<HSTRING>, IEquatable<HSTRING>, IFormattable
{
    public readonly void* Value;

    public HSTRING(void* value)
    {
        Value = value;
    }

    public static HSTRING INVALID_VALUE => new HSTRING((void*)(-1));

    public static HSTRING NULL => new HSTRING(null);

    public static bool operator ==(HSTRING left, HSTRING right) => left.Value == right.Value;

    public static bool operator !=(HSTRING left, HSTRING right) => left.Value != right.Value;

    public static bool operator <(HSTRING left, HSTRING right) => left.Value < right.Value;

    public static bool operator <=(HSTRING left, HSTRING right) => left.Value <= right.Value;

    public static bool operator >(HSTRING left, HSTRING right) => left.Value > right.Value;

    public static bool operator >=(HSTRING left, HSTRING right) => left.Value >= right.Value;

    public static explicit operator HSTRING(void* value) => new HSTRING(value);

    public static implicit operator void*(HSTRING value) => value.Value;

    public static explicit operator HSTRING(byte value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator byte(HSTRING value) => (byte)(value.Value);

    public static explicit operator HSTRING(short value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator short(HSTRING value) => (short)(value.Value);

    public static explicit operator HSTRING(int value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator int(HSTRING value) => (int)(value.Value);

    public static explicit operator HSTRING(long value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator long(HSTRING value) => (long)(value.Value);

    public static explicit operator HSTRING(nint value) => new HSTRING(unchecked((void*)(value)));

    public static implicit operator nint(HSTRING value) => (nint)(value.Value);

    public static explicit operator HSTRING(sbyte value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator sbyte(HSTRING value) => (sbyte)(value.Value);

    public static explicit operator HSTRING(ushort value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator ushort(HSTRING value) => (ushort)(value.Value);

    public static explicit operator HSTRING(uint value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator uint(HSTRING value) => (uint)(value.Value);

    public static explicit operator HSTRING(ulong value) => new HSTRING(unchecked((void*)(value)));

    public static explicit operator ulong(HSTRING value) => (ulong)(value.Value);

    public static explicit operator HSTRING(nuint value) => new HSTRING(unchecked((void*)(value)));

    public static implicit operator nuint(HSTRING value) => (nuint)(value.Value);

    public int CompareTo(object? obj)
    {
        if (obj is HSTRING other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException("obj is not an instance of HSTRING.");
    }

    public int CompareTo(HSTRING other) => ((nuint)(Value)).CompareTo((nuint)(other.Value));

    public override bool Equals(object? obj) => (obj is HSTRING other) && Equals(other);

    public bool Equals(HSTRING other) => ((nuint)(Value)).Equals((nuint)(other.Value));

    public override int GetHashCode() => ((nuint)(Value)).GetHashCode();

    public override string ToString() => ((nuint)(Value)).ToString((sizeof(nint) == 4) ? "X8" : "X16");

    public string ToString(string? format, IFormatProvider? formatProvider) => ((nuint)(Value)).ToString(format, formatProvider);
}

public enum TrustLevel
{
    /// <include file='TrustLevel.xml' path='doc/member[@name="TrustLevel.BaseTrust"]/*' />
    BaseTrust = 0,

    /// <include file='TrustLevel.xml' path='doc/member[@name="TrustLevel.PartialTrust"]/*' />
    PartialTrust = (BaseTrust + 1),

    /// <include file='TrustLevel.xml' path='doc/member[@name="TrustLevel.FullTrust"]/*' />
    FullTrust = (PartialTrust + 1),
}

[Guid("AF86E2E0-B12D-4C6A-9C5A-D7AA65101E90")]
[SupportedOSPlatform("windows6.2")]
public unsafe partial struct IInspectable : IInspectable.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IInspectable);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IInspectable*, Guid*, void**, int>)(lpVtbl[0]))((IInspectable*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IInspectable*, uint>)(lpVtbl[1]))((IInspectable*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IInspectable*, uint>)(lpVtbl[2]))((IInspectable*)Unsafe.AsPointer(ref this));
    }

    /// <include file='IInspectable.xml' path='doc/member[@name="IInspectable.GetIids"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetIids(uint* iidCount, Guid** iids)
    {
        return ((delegate* unmanaged[MemberFunction]<IInspectable*, uint*, Guid**, int>)(lpVtbl[3]))((IInspectable*)Unsafe.AsPointer(ref this), iidCount, iids);
    }

    /// <include file='IInspectable.xml' path='doc/member[@name="IInspectable.GetRuntimeClassName"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetRuntimeClassName(HSTRING* className)
    {
        return ((delegate* unmanaged[MemberFunction]<IInspectable*, HSTRING*, int>)(lpVtbl[4]))((IInspectable*)Unsafe.AsPointer(ref this), className);
    }

    /// <include file='IInspectable.xml' path='doc/member[@name="IInspectable.GetTrustLevel"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetTrustLevel(TrustLevel* trustLevel)
    {
        return ((delegate* unmanaged[MemberFunction]<IInspectable*, TrustLevel*, int>)(lpVtbl[5]))((IInspectable*)Unsafe.AsPointer(ref this), trustLevel);
    }

    public interface Interface : IUnknown.Interface
    {
        HRESULT GetIids(uint* iidCount, Guid** iids);

        HRESULT GetRuntimeClassName(HSTRING* className);

        HRESULT GetTrustLevel(TrustLevel* trustLevel);
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        public delegate* unmanaged[MemberFunction]<TSelf*, Guid*, void**, int> QueryInterface;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> AddRef;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> Release;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint*, Guid**, int> GetIids;

        public delegate* unmanaged[MemberFunction]<TSelf*, HSTRING*, int> GetRuntimeClassName;

        public delegate* unmanaged[MemberFunction]<TSelf*, TrustLevel*, int> GetTrustLevel;
    }
}

[Guid("A4ED5C81-76C9-40BD-8BE6-B1D90FB20AE7")]
public unsafe partial struct IAsyncActionCompletedHandler : IAsyncActionCompletedHandler.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IAsyncActionCompletedHandler);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncActionCompletedHandler*, Guid*, void**, int>)(lpVtbl[0]))((IAsyncActionCompletedHandler*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncActionCompletedHandler*, uint>)(lpVtbl[1]))((IAsyncActionCompletedHandler*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncActionCompletedHandler*, uint>)(lpVtbl[2]))((IAsyncActionCompletedHandler*)Unsafe.AsPointer(ref this));
    }

    /// <include file='IAsyncActionCompletedHandler.xml' path='doc/member[@name="IAsyncActionCompletedHandler.Invoke"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT Invoke(IAsyncAction* asyncInfo, AsyncStatus asyncStatus)
    {
        return ((delegate* unmanaged[MemberFunction]<IAsyncActionCompletedHandler*, IAsyncAction*, AsyncStatus, int>)(lpVtbl[3]))((IAsyncActionCompletedHandler*)Unsafe.AsPointer(ref this), asyncInfo, asyncStatus);
    }

    public interface Interface : IUnknown.Interface
    {
        HRESULT Invoke(IAsyncAction* asyncInfo, AsyncStatus asyncStatus);
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        public delegate* unmanaged[MemberFunction]<TSelf*, Guid*, void**, int> QueryInterface;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> AddRef;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> Release;

        public delegate* unmanaged[MemberFunction]<TSelf*, IAsyncAction*, AsyncStatus, int> Invoke;
    }
}

[Guid("60DB2FA8-B705-4FDE-A7D6-EBBB1891D39E")]
public unsafe partial struct ICoreDispatcher : ICoreDispatcher.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_ICoreDispatcher);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, Guid*, void**, int>)(lpVtbl[0]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, uint>)(lpVtbl[1]))((ICoreDispatcher*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, uint>)(lpVtbl[2]))((ICoreDispatcher*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IInspectable.GetIids" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetIids(uint* iidCount, Guid** iids)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, uint*, Guid**, int>)(lpVtbl[3]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), iidCount, iids);
    }

    /// <inheritdoc cref="IInspectable.GetRuntimeClassName" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetRuntimeClassName(HSTRING* className)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, HSTRING*, int>)(lpVtbl[4]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), className);
    }

    /// <inheritdoc cref="IInspectable.GetTrustLevel" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT GetTrustLevel(TrustLevel* trustLevel)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, TrustLevel*, int>)(lpVtbl[5]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), trustLevel);
    }

    /// <include file='ICoreDispatcher.xml' path='doc/member[@name="ICoreDispatcher.get_HasThreadAccess"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT get_HasThreadAccess(byte* value)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, byte*, int>)(lpVtbl[6]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), value);
    }

    /// <include file='ICoreDispatcher.xml' path='doc/member[@name="ICoreDispatcher.ProcessEvents"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT ProcessEvents(CoreProcessEventsOption options)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, CoreProcessEventsOption, int>)(lpVtbl[7]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), options);
    }

    /// <include file='ICoreDispatcher.xml' path='doc/member[@name="ICoreDispatcher.RunAsync"]/*' />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT RunAsync(CoreDispatcherPriority priority, IDispatchedHandler* agileCallback, IAsyncAction** asyncAction)
    {
        return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, CoreDispatcherPriority, IDispatchedHandler*, IAsyncAction**, int>)(lpVtbl[8]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), priority, agileCallback, asyncAction);
    }

    /// <include file='ICoreDispatcher.xml' path='doc/member[@name="ICoreDispatcher.RunIdleAsync"]/*' />
    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //public HRESULT RunIdleAsync(IIdleDispatchedHandler* agileCallback, IAsyncAction** asyncAction)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ICoreDispatcher*, IIdleDispatchedHandler*, IAsyncAction**, int>)(lpVtbl[9]))((ICoreDispatcher*)Unsafe.AsPointer(ref this), agileCallback, asyncAction);
    //}

    public interface Interface : IInspectable.Interface
    {
        HRESULT get_HasThreadAccess(byte* value);

        HRESULT ProcessEvents(CoreProcessEventsOption options);

        HRESULT RunAsync(CoreDispatcherPriority priority, IDispatchedHandler* agileCallback, IAsyncAction** asyncAction);

        //HRESULT RunIdleAsync(IIdleDispatchedHandler* agileCallback, IAsyncAction** asyncAction);
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        public delegate* unmanaged[MemberFunction]<TSelf*, Guid*, void**, int> QueryInterface;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> AddRef;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint> Release;

        public delegate* unmanaged[MemberFunction]<TSelf*, uint*, Guid**, int> GetIids;

        public delegate* unmanaged[MemberFunction]<TSelf*, HSTRING*, int> GetRuntimeClassName;

        public delegate* unmanaged[MemberFunction]<TSelf*, TrustLevel*, int> GetTrustLevel;

        public delegate* unmanaged[MemberFunction]<TSelf*, byte*, int> get_HasThreadAccess;

        public delegate* unmanaged[MemberFunction]<TSelf*, CoreProcessEventsOption, int> ProcessEvents;

        public delegate* unmanaged[MemberFunction]<TSelf*, CoreDispatcherPriority, IDispatchedHandler*, IAsyncAction**, int> RunAsync;

        //public delegate* unmanaged[MemberFunction]<TSelf*, IIdleDispatchedHandler*, IAsyncAction**, int> RunIdleAsync;
    }
}