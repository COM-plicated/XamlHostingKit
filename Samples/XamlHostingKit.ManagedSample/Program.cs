namespace XamlHostingKit.ManagedSample
{
    internal class Program
    {
        [STAThread]
        static void Main()
        {
            XamlApplication.Start((p) => new App());
        }
    }
}
