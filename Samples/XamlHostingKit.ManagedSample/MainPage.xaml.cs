using Windows.Foundation.Metadata;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace XamlHostingKit.ManagedSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private static readonly bool IsWebViewExecutionModeAvailable = ((Func<bool>)(() =>
        {
            try
            {
                return ApiInformation.IsTypePresent("Windows.UI.Xaml.Controls.WebViewExecutionMode");
            }
            catch
            {
                return false;
            }
        }))();

        public MainPage()
        {
            this.InitializeComponent();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            XamlWindow.Current.TitleBar.SetTitleBar(TitleBar);

            WebView wv = IsWebViewExecutionModeAvailable ? new(WebViewExecutionMode.SeparateThread) : new();
            //wv.Source = new("ms-appx-web:///test.html"),
            wv.Source = new("https://gist.github.com/user-attachments/assets/3e03fe73-4af4-48d7-ade6-a2af4403dbb7");
            wv.VerticalAlignment = VerticalAlignment.Center;
            wv.HorizontalAlignment = HorizontalAlignment.Center;
            wv.Width = 400;
            wv.Height = 400;
            wv.Margin = new Thickness(0, 20, 0, 0);

            // For Win8.1:
            //wv.NavigateToString(
            //    """
            //    <html>
            //    <head>
            //        <style>
            //            html, body { margin: 0; padding: 0; width: 100%; height: 100%; overflow: hidden; background: #000; }
            //            video { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
            //        </style>
            //    </head>
            //    <body>
            //        <video src="https://gist.github.com/user-attachments/assets/3e03fe73-4af4-48d7-ade6-a2af4403dbb7" autoplay controls></video>
            //    </body>
            //    </html>
            //    """);

            panel.Children.Add(wv);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            XamlApplication.CreateWindow(new() { Title = "XamlHostingKit Secondary Window" }, (p) =>
            {
                var window = XamlWindow.Current;

                Frame frame = new();
                frame.Navigate(typeof(SecondaryPage));
                window.Content = frame;

                window.Show();
            });
        }
    }
}
