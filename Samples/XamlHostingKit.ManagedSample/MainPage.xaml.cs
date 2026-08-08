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
        public MainPage()
        {
            this.InitializeComponent();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            WebView wv = new(WebViewExecutionMode.SeparateThread)
            {
                //Source = new Uri("ms-appx-web:///test.html"),
                Source = new Uri("https://gist.github.com/user-attachments/assets/3e03fe73-4af4-48d7-ade6-a2af4403dbb7"),
                VerticalAlignment = VerticalAlignment.Center,
                HorizontalAlignment = HorizontalAlignment.Center,
                Width = 400,
                Height = 400,
                Margin = new Thickness(0, 20, 0, 0)
            };

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
