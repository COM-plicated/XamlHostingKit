using WinRT;
using System;
using WinRT.Interop;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Diagnostics.CodeAnalysis;

namespace XamlHostingKit.Static.Projection
{
    internal static unsafe partial class ProjectionModuleInitializer
    {
        private static bool _isCoreCLR = RuntimeFeature.IsDynamicCodeCompiled;

        [SuppressMessage("Interoperability", "SYSLIB1054")]
        [DllImport("XamlHostingKit.Static", EntryPoint = "XHK_GetActivationFactory")]
        internal static extern int XHK_GetActivationFactory(void* classId, void** factory);

        [ModuleInitializer]
        [SuppressMessage("Usage", "CA2255")]
        internal static void InitializeProjection()
        {
            var originalHandler = ActivationFactory.ActivationHandler;
            ActivationFactory.ActivationHandler = (name, iid) =>
            {
                nint result = 0;
                MarshalString? classId = null;

                try
                {
                    if (originalHandler is not null && ((result = originalHandler(name, iid)) is not 0))
                        return result;
                }
                catch
                {
                    // nothing to do here...
                }

                if (!_isCoreCLR)
                {
                    IUnknownVftbl** vftbl = null;

                    try
                    {
                        classId = MarshalString.CreateMarshaler(name);
                        if (classId is not null)
                        {
                            if (XHK_GetActivationFactory((void*)classId.GetAbi(), (void**)&vftbl) == 0 &&
                                (*vftbl)->QueryInterface((nint)vftbl, &iid, &result) == 0)
                            {
                                return result;
                            }
                        }
                    }
                    catch (DllNotFoundException)
                    {
                        _isCoreCLR = true;
                        OutputDebugStringW("[XamlHostingKit] WARNING: Cannot import XHK_GetActivationFactory, you can safely ignore this warning on non-NativeAOT and/or non-Publish builds.\r\n");
                    }
                    catch
                    {
                        // nothing to do here...
                    }
                    finally
                    {
                        if (vftbl is not null)
                        {
                            (*vftbl)->Release((nint)vftbl);
                        }

                        if (classId is not null)
                        {
                            classId.Dispose();
                            classId = null;
                        }
                    }
                }

                return result;
            };
        }

        [DefaultDllImportSearchPaths(DllImportSearchPath.System32)]
        [LibraryImport("kernel32.dll", StringMarshalling = StringMarshalling.Utf16)]
        private static partial void OutputDebugStringW(string message);
    }
}
