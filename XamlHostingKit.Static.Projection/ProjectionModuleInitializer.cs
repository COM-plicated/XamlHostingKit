using WinRT;
using System;
using WinRT.Interop;
using System.Runtime.InteropServices;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace XamlHostingKit.Static.Projection
{
    internal static unsafe class ProjectionModuleInitializer
    {
        [SuppressMessage("Interoperability", "SYSLIB1054")]
        [DllImport("XamlHostingKit.Static", EntryPoint = "XHK_GetActivationFactory")]
        internal static extern int XHK_GetActivationFactory(void* classId, void** factory);

        [ModuleInitializer]
        [SuppressMessage("Usage", "CA2255")]
        internal static void InitializeProjection()
        {
            var originalHandler = ActivationFactory.ActivationHandler;
            ActivationFactory.ActivationHandler = (string name, Guid iid) =>
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

                try
                {
                    classId = MarshalString.CreateMarshaler(name);
                    if (classId is not null)
                    {
                        IUnknownVftbl** vftbl = null;
                        if (XHK_GetActivationFactory((void*)classId.GetAbi(), (void**)&vftbl) == 0 &&
                            (*vftbl)->QueryInterface((nint)vftbl, &iid, &result) == 0)
                        {
                            return result;
                        }
                    }
                }
                catch
                {
                    // nothing to do here...
                }
                finally
                {
                    if (classId is not null)
                    {
                        classId.Dispose();
                        classId = null;
                    }
                }

                return result;
            };
        }
    }
}
