using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace CodeCraft2019Visualization
{
    partial class Scenario
    {
        private void DoInitialize()
        {
            m_roads.Clear();
            m_crosses.Clear();
            m_cars.Clear();
            m_datas.Clear();
            m_basic = null;
            m_images.Clear();
            m_bytes.Clear();
            InitilizeScenario();
            InitializeBasicImage();
            DrawBasicImage();
            m_ioThread.Start();
            m_drawingThread.Start();
        }

        private static Regex LineRegex = new Regex(@"\(([,\-\d]+)\)", RegexOptions.Compiled);
        private List<int> ReadLine(string str)
        {
            if (str.Length <= 0) return null;
            int pos = str.IndexOf('#');
            if (pos >= 0)
                str = str.Substring(0, pos);
            str = str.Replace(" ", "");
            if (str.Length <= 0) return null;
            var match = LineRegex.Match(str);
            Logging.Assert(match.Groups.Count == 2);
            return match.Groups[1].Value.Split(',')
                .Aggregate(new List<int>(), (l, s) => { l.Add(Convert.ToInt32(s)); return l; });
        }

        private void InitilizeScenario()
        {
            Logging.Log("read file : " + Config.Path.ConfigDir + Config.Path.CarName);
            using (var fs = new FileStream(Config.Path.ConfigDir + Config.Path.CarName, FileMode.Open))
            {
                if (fs == null) return;
                using (var sr = new StreamReader(fs))
                {
                    while (!sr.EndOfStream)
                    {
                        var ret = ReadLine(sr.ReadLine());
                        if (ret == null) continue;
                        Logging.Assert(ret.Count == 5 || ret.Count == 7);
                        int id = ret[0];
                        Car car = new Car();
                        car.Id = id;
                        if (ret.Count == 7 && ret[5] == 1)
                            car.IsVip = true;
                        if (ret.Count == 7 && ret[6] == 1)
                            car.Type = Car.CarType.Preset;
                        m_cars.Add(id, car);
                    }
                }
            }

            Logging.Log("read file : " + Config.Path.ConfigDir + Config.Path.CrossName);
            using (var fs = new FileStream(Config.Path.ConfigDir + Config.Path.CrossName, FileMode.Open))
            {
                if (fs == null) return;
                using (var sr = new StreamReader(fs))
                {
                    while (!sr.EndOfStream)
                    {
                        var ret = ReadLine(sr.ReadLine());
                        if (ret == null) continue;
                        Logging.Assert(ret.Count == 5);
                        int id = ret[0];
                        Cross cross = new Cross();
                        cross.Id = id;
                        for (int i = 0; i < 4; ++i)
                            cross.Neighbors[i] = ret[i + 1];
                        m_crosses.Add(id, cross);
                    }
                }
            }

            Logging.Log("read file : " + Config.Path.ConfigDir + Config.Path.RoadName);
            using (var fs = new FileStream(Config.Path.ConfigDir + Config.Path.RoadName, FileMode.Open))
            {
                if (fs == null) return;
                using (var sr = new StreamReader(fs))
                {
                    while (!sr.EndOfStream)
                    {
                        var ret = ReadLine(sr.ReadLine());
                        if (ret == null) continue;
                        Logging.Assert(ret.Count == 7);
                        int id = ret[0];
                        Dictionary<bool, Road> roads = new Dictionary<bool, Road>();
                        Road roadForward = new Road();
                        roadForward.Id = id;
                        roadForward.Length = ret[1];
                        roadForward.Lanes = ret[3];
                        roadForward.From = m_crosses[ret[4]];
                        roadForward.To = m_crosses[ret[5]];
                        roads.Add(true, roadForward);
                        if (ret[6] == 1)
                        {
                            Road roadBackward = new Road();
                            roadBackward.Id = id;
                            roadBackward.Length = ret[1];
                            roadBackward.Lanes = ret[3];
                            roadBackward.From = m_crosses[ret[5]];
                            roadBackward.To = m_crosses[ret[4]];
                            roads.Add(false, roadBackward);
                        }
                        m_roads.Add(id, roads);
                    }
                }
            }
        }

        private void InitializeBasicImage()
        {
            Logging.Log("initializ scenario image");
            int minX = 0, minY = 0, maxX = 0, maxY = 0;
            Action<Cross> initCross = null;
            initCross = new Action<Cross>(cross =>
            {
                for (int index = 0; index < cross.Neighbors.Length; ++index)
                {
                    int id = cross.Neighbors[index];
                    if (id <= 0) continue;
                    Road road = m_roads[id][true];
                    Cross neighbor = null;
                    if (road.From == cross)
                        neighbor = road.To;
                    else
                        neighbor = road.From;
                    if (neighbor.IsInitilized) continue;
                    int newX = cross.Position.X + (index % 2 == 1 ? (index == 1 ? 1 : -1) : 0);
                    int newY = cross.Position.Y + (index % 2 == 0 ? (index == 0 ? -1 : 1) : 0);
                    if (newX < minX) minX = newX;
                    if (newX > maxX) maxX = newX;
                    if (newY < minY) minY = newY;
                    if (newY > maxY) maxY = newY;
                    neighbor.AnchorByNeighbor(newX, newY, cross, road, index);
                    initCross(neighbor);
                }
            });
            m_crosses.First().Value.Anchor(0, 0);
            initCross(m_crosses.First().Value);

            foreach (var cross in m_crosses.Values)
            {
                cross.Anchor((cross.Position.X - minX + 1) * Config.Size.IntervalBetweenCross
                    , (cross.Position.Y - minY + 1) * Config.Size.IntervalBetweenCross);
            }
            foreach (var roads in m_roads.Values)
            {
                //continue;
                foreach (var v in roads)
                {
                    bool forward = v.Key;
                    Road road = v.Value;
                    Logging.Assert(road.From.Position.X == road.To.Position.X
                        || road.From.Position.Y == road.To.Position.Y);
                    int autoLength = road.Lanes * Config.Size.CarWidth;
                    int margin = 2;
                    int padding = Config.Size.RoadPadding;
                    if (road.From.Position.Y == road.To.Position.Y)
                    {
                        if (road.From.Position.X < road.To.Position.X)
                        {
                            road.Direction = Road.DrawDirection.Right;
                            road.Position.Location = new Point(road.From.Position.X + padding, road.From.Position.Y + margin);
                            road.Position.Size = new Size(road.To.Position.X - road.From.Position.X - 2 * padding, autoLength);
                        }
                        else
                        {
                            road.Direction = Road.DrawDirection.Left;
                            road.Position.Location = new Point(road.To.Position.X + padding, road.To.Position.Y - autoLength - margin);
                            road.Position.Size = new Size(road.From.Position.X - road.To.Position.X - 2 * padding, autoLength);
                        }
                    }
                    else
                    {
                        if (road.From.Position.Y < road.To.Position.Y)
                        {
                            road.Direction = Road.DrawDirection.Down;
                            road.Position.Location = new Point(road.From.Position.X - autoLength - margin, road.From.Position.Y + padding);
                            road.Position.Size = new Size(autoLength, road.To.Position.Y - road.From.Position.Y - 2 * padding);
                        }
                        else
                        {
                            road.Direction = Road.DrawDirection.Up;
                            road.Position.Location = new Point(road.To.Position.X + margin, road.To.Position.Y + padding);
                            road.Position.Size = new Size(autoLength, road.From.Position.Y - road.To.Position.Y - 2 * padding);
                        }
                    }
                }
            }

            int width = (maxX - minX + 2) * Config.Size.IntervalBetweenCross;
            int height = (maxY - minY + 2) * Config.Size.IntervalBetweenCross;
            m_basic = new System.Drawing.Bitmap(width, height);
            Logging.Log("image size : " + width + " " + height);
        }

        private void DrawBasicImage()
        {
            Logging.Log("generate basic image");
            if (m_basic == null) return;
            using (Graphics graphics = Graphics.FromImage(m_basic))
            {
                foreach (var cross in m_crosses.Values)
                    cross.Draw(graphics);
                foreach (var roads in m_roads.Values)
                {
                    foreach (var road in roads.Values)
                        road.Draw(graphics);
                    if (roads.Count == 1)
                        roads.First().Value.DrawRoadId(graphics);
                    else if (roads.Count == 2)
                        foreach (var road in roads.Values)
                            if (road.Direction == Road.DrawDirection.Right || road.Direction == Road.DrawDirection.Up)
                                road.DrawRoadId(graphics);
                }
            }
            if (Config.Parameter.SaveImage)
                m_basic.Save(Config.Path.OutputDir + "basic.jpg");
        }
    }
}
