namespace XamlHostingKit.ManagedSample
{
    internal class Program
    {
        [STAThread]
        static void Main()
        {
            //XamlConfig.EnableMsAppxWebProtocolSupport = true;
            //XamlConfig.EnableArbitraryPathsInMsAppxWeb = true;
            XamlApplication.Start((p) => new App());
        }
    }
}
