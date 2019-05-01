using System.Collections.Generic;
using System.Drawing;
using System.Threading;
using System.Windows.Media;

namespace CodeCraft2019Visualization
{
    partial class Scenario
    {
        Dictionary<int, Dictionary<bool, Road>> m_roads = new Dictionary<int, Dictionary<bool, Road>>();
        Dictionary<int, Cross> m_crosses = new Dictionary<int, Cross>();
        Dictionary<int, Car> m_cars = new Dictionary<int, Car>();
        List<TimeChip> m_datas = new List<TimeChip>();

        Bitmap m_basic = null;
        public System.Windows.Threading.Dispatcher UIDispatcher { get; set; } = null;
        List<KeyValuePair<int, byte[]>> m_bytes = new List<KeyValuePair<int, byte[]>>();
        List<KeyValuePair<int, System.Windows.Media.Imaging.BitmapImage>> m_images = new List<KeyValuePair<int, System.Windows.Media.Imaging.BitmapImage>>();

        Thread m_initThread = null;
        Thread m_drawingThread = null;
        Thread m_ioThread = null;

        public int TimeChipsN => m_datas.Count;
        public int ImageN => Config.Parameter.SaveMemory ? m_bytes.Count : m_images.Count;
        public bool IsDrawingComplete { get; private set; } = false;
        public bool IsIOComplete { get; private set; } = false;
        public KeyValuePair<int, System.Windows.Media.Imaging.BitmapImage> Image(int index) => m_images[index];
        public KeyValuePair<int, byte[]> ImageBytes(int index) => m_bytes[index];
        public Bitmap BasicImage => m_basic;
        public bool CrossValid(int index) => m_crosses.ContainsKey(index);
        public bool RoadValid(int index) => m_roads.ContainsKey(index);
        public Point CrossPosition(int index) => m_crosses[index].Position;
        public Rectangle RoadPosition(int index) => m_roads[index][true].Position;

        public Scenario() { }

        public void Initialize()
        {
            Dispose();
            IsDrawingComplete = false;
            IsIOComplete = false;
            m_initThread = new Thread(new ThreadStart(DoInitialize));
            m_drawingThread = new Thread(new ThreadStart(Drawing));
            m_ioThread = new Thread(new ThreadStart(IO));
            m_initThread.Start();
        }

        public void Dispose()
        {
            m_initThread?.Abort();
            m_ioThread?.Abort();
            m_drawingThread?.Abort();
            IsDrawingComplete = true;
            IsIOComplete = true;
        }
    }
}
