using System.Runtime.InteropServices;

namespace XamlHostingKit.ManagedSample
{
    internal partial class Program
    {
        //[LibraryImport("kernel.appcore.dll", StringMarshalling = StringMarshalling.Utf16)]
        //public static partial int AddDependencyToProcessPackageGraph(string packageFamilyName, nint unk = 0, uint unk2 = 0, uint unk3 = 0);

        [STAThread]
        static void Main()
        {
            //XamlConfig.EnableMsAppxWebProtocolSupport = true;
            //XamlConfig.EnableArbitraryPathsInMsAppxWeb = true;
            //var hr = AddDependencyToProcessPackageGraph("Microsoft.UI.Xaml.2.8_8wekyb3d8bbwe");
            XamlApplication.Start((p) => new App());
        }
    }
}
