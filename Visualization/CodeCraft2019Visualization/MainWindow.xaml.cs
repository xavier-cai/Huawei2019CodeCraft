using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace CodeCraft2019Visualization
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            this.Width = Config.Parameter.WindowSize.Width;
            this.Height = Config.Parameter.WindowSize.Height;
        }

        Scenario m_scenario;
        DispatcherTimer m_refreshTimer;
        int m_index = -1;

        DispatcherTimer m_playTimer;

        private void WindowLoaded(object sender, RoutedEventArgs e)
        {
            m_scenario = new Scenario();
            m_scenario.UIDispatcher = Dispatcher;

            m_refreshTimer = new DispatcherTimer();
            m_refreshTimer.Tick += Refresh;
            m_refreshTimer.Interval = TimeSpan.FromSeconds(0.1);
            m_refreshTimer.Start();

            m_playTimer = new DispatcherTimer();
            m_playTimer.Tick += AutoPlay;
            m_playTimer.Interval = TimeSpan.FromSeconds(Config.Parameter.PlayInterval);

            ctlTextInterval.Text = Config.Parameter.PlayInterval.ToString();

            LoadScenario();
        }

        private void WindowClosed(object sender, EventArgs e)
        {
            m_scenario.Dispose();
            Logging.Log("save configure");
            Config.Parameter.WindowSize = new System.Drawing.Size((int)this.Width, (int)this.Height);
            Config.Parameter.PlayInterval = m_playTimer.Interval.Milliseconds / 1e3;
            Config.SaveConfigure();
        }

        private void LoadScenario()
        {
            m_scenario.Initialize();
        }

        private void Refresh(object sender, EventArgs e)
        {
            string str;
            if (m_scenario.IsIOComplete && m_scenario.IsDrawingComplete)
                str = "Completed";
            else if (m_scenario.IsIOComplete)
                str = "IO Completed";
            else
                str = "IO Uncomplete";
            ctlLabel.Content = str + " " + m_scenario.ImageN + "/" + m_scenario.TimeChipsN + " IMG/IO";

            if (m_index < 0 && m_scenario.ImageN > 0)
            {
                RefreshImage(0);
            }
        }

        ImageSourceConverter imageSourceConverter = new ImageSourceConverter();
        private void RefreshImage(int index)
        {
            if (index < 0 || index >= m_scenario.ImageN)
                return;
            m_index = index;
            Logging.Assert(m_index >= 0 && m_index < m_scenario.ImageN);
            int key = -1;
            if (Config.Parameter.SaveMemory)
            {
                var image = m_scenario.ImageBytes(m_index);
                ctlImage.Source = imageSourceConverter.ConvertFrom(new MemoryStream(image.Value)) as ImageSource;
                key = image.Key;
            }
            else
            {
                var image = m_scenario.Image(m_index);
                ctlImage.Source = image.Value;
                key = image.Key;
            }
            ctlIndexLabel.Content = "T/C " + key + "/" + m_index;
        }

        private void ButtonLeftClick(object sender, RoutedEventArgs e)
        {
            if (sender != m_playTimer)
            {
                if (m_playTimer.IsEnabled)
                    m_playTimer.Stop();
            }
            if (m_index > 0 && (m_index - 1) < m_scenario.ImageN)
            {
                RefreshImage(m_index - 1);
            }
        }

        private void ButtonRightClick(object sender, RoutedEventArgs e)
        {
            if (sender != m_playTimer)
            {
                if (m_playTimer.IsEnabled)
                    m_playTimer.Stop();
            }
            if (m_index >= 0 && m_index < m_scenario.ImageN - 1)
            {
                RefreshImage(m_index + 1);
            }
        }

        private void ButtonPlayPauseClick(object sender, RoutedEventArgs e)
        {
            if (m_playTimer.IsEnabled)
                m_playTimer.Stop();
            else
            {
                try
                {
                    double interval = Convert.ToDouble(ctlTextInterval.Text);
                    m_playTimer.Interval = TimeSpan.FromSeconds(interval);
                    m_playTimer.Start();
                }
                catch
                {
                    MessageBox.Show("Play failed : Invalid timer interval [" + ctlTextInterval.Text + "]");
                }
            }
        }

        private void ButtonJumpChipClick(object sender, RoutedEventArgs e)
        {
            try
            {
                int jump = Convert.ToInt32(ctlTextJump.Text);
                if (jump < 0 || jump >= m_scenario.ImageN)
                    MessageBox.Show("Jump failed : Invalid jump target chip [" + ctlTextJump.Text + "]");
                else
                    RefreshImage(jump);
            }
            catch
            {
                MessageBox.Show("Jump failed : Invalid jump target chip [" + ctlTextJump.Text + "]");
            }
        }

        private void ButtonJumpCrossClick(object sender, RoutedEventArgs e)
        {
            try
            {
                int jump = Convert.ToInt32(ctlTextJump.Text);
                if (!m_scenario.CrossValid(jump))
                    MessageBox.Show("Jump failed : Invalid jump target cross [" + ctlTextJump.Text + "]");
                else
                    MoveViewTo(m_scenario.CrossPosition(jump));
            }
            catch
            {
                MessageBox.Show("Jump failed : Invalid jump target cross [" + ctlTextJump.Text + "]");
            }
        }

        private void ButtonJumpRoadClick(object sender, RoutedEventArgs e)
        {
            try
            {
                int jump = Convert.ToInt32(ctlTextJump.Text);
                if (!m_scenario.RoadValid(jump))
                    MessageBox.Show("Jump failed : Invalid jump target road [" + ctlTextJump.Text + "]");
                else
                {
                    var rect = m_scenario.RoadPosition(jump);
                    MoveViewTo(new System.Drawing.Point(rect.X + rect.Width / 2, rect.Y + rect.Height / 2));
                }
            }
            catch
            {
                MessageBox.Show("Jump failed : Invalid jump target road [" + ctlTextJump.Text + "]");
            }
        }

        private void ButtonDisposeClick(object sender, RoutedEventArgs e)
        {
            if (m_scenario.IsDrawingComplete && m_scenario.IsIOComplete)
            {
                m_scenario.Initialize();
                m_index = -1;
            }
            else
                m_scenario.Dispose();
        }

        private void AutoPlay(object sender, EventArgs e)
        {
            int index = m_index;
            ButtonRightClick(sender, null);
            if (index == m_index)
                m_playTimer.Stop();
        }

        private void ButtonResetViewClick(object sender, RoutedEventArgs e)
        {
            var state = (ctlImage.RenderTransform as TransformGroup).CloneCurrentValue();
            var translater = state.Children[0] as TranslateTransform;
            var scaler = state.Children[1] as ScaleTransform;
            translater.X = 0;
            translater.Y = 0;
            scaler.ScaleX = 1;
            scaler.ScaleY = 1;
            scaler.CenterX = 0;
            scaler.CenterY = 0;
            ctlImage.RenderTransform = state;
        }

        private void WindowMouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (e.Delta == 0) return;
            
            var state = (ctlImage.RenderTransform as TransformGroup).CloneCurrentValue();
            //var translater = state.Children[0] as TranslateTransform;
            var scaler = state.Children[1] as ScaleTransform;

            double currentDelta = scaler.ScaleX;
            double delta = (e.Delta > 0 ? 1 : -1) * (currentDelta / 2.0 * 0.1);
            if ((delta > 0 && scaler.ScaleX < 8.0)
                || (delta < 0 && scaler.ScaleX > 0.99))
            {
                scaler.ScaleX += delta;
                scaler.ScaleY += delta;
                scaler.CenterX = ctlImage.ActualWidth / 2;
                scaler.CenterY = ctlImage.ActualHeight / 2;
            }
            ctlImage.RenderTransform = state;
        }

        private void WindowKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Left: ButtonLeftClick(sender, null); e.Handled = true; break;
                case Key.Right: ButtonRightClick(sender, null); e.Handled = true; break;
                case Key.Space: ButtonPlayPauseClick(sender, null); e.Handled = true; break;
            }
        }

        bool isMoving = false;
        Point clickPosition;
        Point initTransform;
        private void ImageMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            ctlImage.Focus();
            if (sender == ctlImage && !isMoving)
            {
                isMoving = true;
                clickPosition = e.GetPosition(ctlImageContainer);
                var translate = (((sender as UIElement).RenderTransform as TransformGroup).Children[0] as TranslateTransform);
                initTransform = new Point(translate.X, translate.Y);
                (sender as UIElement).CaptureMouse();
            }
        }

        private void ImageMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (sender == ctlImage && isMoving)
            {
                isMoving = false;
                (sender as UIElement).ReleaseMouseCapture();
            }
        }

        private void ImageMouseMove(object sender, MouseEventArgs e)
        {
            if (sender == ctlImage && isMoving)
            {
                var element = sender as UIElement;
                Point currentPosition = e.GetPosition(ctlImageContainer);
                var state = (element.RenderTransform as TransformGroup).CloneCurrentValue();
                var translater = state.Children[0] as TranslateTransform;
                var scaler = state.Children[1] as ScaleTransform;
                translater.X = initTransform.X + (currentPosition.X - clickPosition.X) / scaler.ScaleX;
                translater.Y = initTransform.Y + (currentPosition.Y - clickPosition.Y) / scaler.ScaleY;
                element.RenderTransform = state;
            }
        }

        private void MoveViewTo(System.Drawing.Point position)
        {
            var state = (ctlImage.RenderTransform as TransformGroup).CloneCurrentValue();
            var translater = state.Children[0] as TranslateTransform;
            translater.X = ctlImage.ActualWidth / 2 - position.X / ctlImage.Source.Width * ctlImage.ActualWidth;
            translater.Y = ctlImage.ActualHeight / 2 - position.Y / ctlImage.Source.Height * ctlImage.ActualHeight;
            ctlImage.RenderTransform = state;
        }
    }
}
