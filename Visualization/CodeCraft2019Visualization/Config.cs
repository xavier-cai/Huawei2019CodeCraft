using System.Drawing;

namespace CodeCraft2019Visualization
{
    static class Config
    {
        static public ConfigPath Path => Instance.Path;
        static public ConfigSize Size => Instance.Size;
        static public ConfigColor Color => Instance.Color;
        static public ConfigParameter Parameter => Instance.Parameter;

        class ConfigContent
        {
            public ConfigContent() { }
            public ConfigPath Path { get; } = new ConfigPath();
            public ConfigSize Size { get; } = new ConfigSize();
            public ConfigColor Color { get; } = new ConfigColor();
            public ConfigParameter Parameter { get; } = new ConfigParameter();
        }

        public class ConfigPath
        {
            public string CarName { get; set; } = "car.txt";
            public string CrossName { get; set; } = "cross.txt";
            public string RoadName { get; set; } = "road.txt";
            public string ConfigDir { get; set; } = "./config/";
            public string LogFullPath { get; set; } = "./log.tr";
            public string OutputDir { get; set; } = "./output/";
        }

        public class ConfigSize
        {
            public int CrossSize { get; set; } = 16;
            public int CarWidth { get; set; } = 4;
            public int RoadPadding { get; set; } = 22;
            public int IntervalBetweenCross { get; set; } = 160;
        }

        public class ConfigColor
        {
            public Color Backgroud = System.Drawing.Color.White;
            public Color Foregroud = System.Drawing.Color.Black;
            public Font Font = new Font("微软雅黑", 12);
            public Color Road = System.Drawing.Color.Gray;
            public Color Cross = System.Drawing.Color.Aqua;
            public Color NormalCar = System.Drawing.Color.Blue;
            public Color PresetCar = System.Drawing.Color.Red;
            public Color ForceCar = System.Drawing.Color.Green;
            public Color VipMark = System.Drawing.Color.Yellow;
        }

        public class ConfigParameter
        {
            public bool SaveMemory { get; set; } = true;
            public bool SaveImage { get; set; } = false;
            public double PlayInterval { get; set; } = 0.2;
            public Size WindowSize { get; set; } = new System.Drawing.Size(750, 500);
        }

        private const string ConfigurePath = "config.ini";
        static private ConfigContent m_instance = null;
        static private ConfigContent Instance
        {
            get
            {
                if (m_instance == null)
                {
                    if (!System.IO.File.Exists(ConfigurePath))
                    {
                        m_instance = new ConfigContent();
                    }
                    else
                    {
                        var read = System.IO.File.ReadAllText(ConfigurePath);
                        m_instance = Newtonsoft.Json.JsonConvert.DeserializeObject<ConfigContent>(read);
                    }
                }
                return m_instance;
            }
        }

        static public void SaveConfigure()
        {
            System.IO.File.WriteAllText(ConfigurePath, Newtonsoft.Json.JsonConvert.SerializeObject(Instance));
        }
    }
}
