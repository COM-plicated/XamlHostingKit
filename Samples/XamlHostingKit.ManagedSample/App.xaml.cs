using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.ApplicationModel.Activation;

namespace XamlHostingKit.ManagedSample
{
    public sealed partial class App : Application
    {
        public App()
        {
            InitializeComponent();
        }

        protected override void OnLaunched(LaunchActivatedEventArgs args)
        {
            var window = XamlWindow.Current;
            window.Title = "XamlHostingKit Managed Sample";

            Frame frame = new();
            frame.Navigate(typeof(MainPage));
            window.Content = frame;

            window.Show();
        }
    }
}
