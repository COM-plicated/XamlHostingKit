using System.Runtime.InteropServices.WindowsRuntime;

namespace XamlHostingKit.ManagedSample
{
    internal class Program
    {
        [STAThread]
        static void Main()
        {
            var asssembly = typeof(Program).Assembly;
            using var embeddedPri = asssembly.GetManifestResourceStream("resources.pri")!;

            var arr = new byte[embeddedPri.Length];
            embeddedPri.ReadExactly(arr);

            XamlApplication.Start((p) => new App(), arr.AsBuffer());
        }
    }
}
