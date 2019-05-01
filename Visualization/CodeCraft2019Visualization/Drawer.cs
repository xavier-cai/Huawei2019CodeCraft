using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Windows.Media.Imaging;

namespace CodeCraft2019Visualization
{
    partial class Scenario
    {
        partial class Cross
        {
            public Point Position = new Point();
            public bool IsInitilized { get; private set; } = false;
            public void Anchor(int x, int y)
            {
                Position.X = x;
                Position.Y = y;
                IsInitilized = true;
            }
            public void AnchorByNeighbor(int x, int y, Cross cross, Road road, int index)
            {
                Logging.Assert(cross.IsInitilized);
                int myIndex = -1;
                for (int i = 0; i < Neighbors.Length; ++i)
                {
                    if (Neighbors[i] == road.Id)
                    {
                        myIndex = i;
                        break;
                    }
                }
                Logging.Assert(myIndex >= 0);
                int shift = ((index + 2) % 4 + 4 - myIndex) % 4;
                int[] origin = new int[Neighbors.Length];
                Neighbors.CopyTo(origin, 0);
                for (int i = 0; i < Neighbors.Length; ++i)
                    Neighbors[i] = origin[(i - shift + 4) % 4];
                Anchor(x, y);
            }
            public void Draw(Graphics graphics)
            {
                graphics.FillEllipse(new SolidBrush(Config.Color.Cross)
                    , new Rectangle(Position.X - Config.Size.CrossSize, Position.Y - Config.Size.CrossSize
                        , Config.Size.CrossSize * 2, Config.Size.CrossSize * 2));
                var str = Id.ToString();
                var size = graphics.MeasureString(str, Config.Color.Font);
                graphics.DrawString(str, Config.Color.Font, new SolidBrush(Config.Color.Foregroud), Position.X - size.Width / 2, Position.Y - size.Height / 2);
            }
        }

        partial class Road
        {
            public Rectangle Position = new Rectangle();
            public DrawDirection Direction { get; set; } = DrawDirection.Down;
            public void DrawCar(Graphics graphics, Car car, int lane, int pos)
            {
                Logging.Assert(lane < Lanes);
                Logging.Assert(pos < Length);
                int x = -1, y = -1;
                switch(Direction)
                {
                    case DrawDirection.Right: x = pos; y = lane; break;
                    case DrawDirection.Left: x = Length - pos - 1; y = Lanes - lane - 1; break;
                    case DrawDirection.Down: x = Lanes - lane - 1; y = pos; break;
                    case DrawDirection.Up: x = lane; y = Length - pos - 1; break;
                }
                bool horizon = Direction == DrawDirection.Left || Direction == DrawDirection.Right;
                float width = horizon ? (Position.Width * 1.0F/ Length) : Config.Size.CarWidth;
                float height = horizon ? Config.Size.CarWidth : (Position.Height * 1.0F/ Length);
                Color color = Config.Color.Foregroud;
                switch (car.Type)
                {
                    case Car.CarType.Normal: color = Config.Color.NormalCar; break;
                    case Car.CarType.Preset: color = Config.Color.PresetCar; break;
                    case Car.CarType.Force: color = Config.Color.ForceCar; break;
                }
                var rect = new RectangleF(Position.X + width * x, Position.Y + height * y, width, height);
                graphics.FillRectangle(new SolidBrush(color), rect);
                if (car.IsVip)
                    graphics.FillRectangle(new SolidBrush(Config.Color.VipMark)
                        , new RectangleF(horizon ? rect.X : (rect.X + rect.Width / 4)
                            , horizon ? (rect.Y + rect.Height / 4) : rect.Y
                            , horizon ? rect.Width : (rect.Width / 2)
                            , horizon ? (rect.Height / 2) : rect.Height));
            }
            public void Draw(Graphics graphics)
            {
                var pen = new Pen(Config.Color.Road, 0.5F);
                //graphics.DrawRectangle(pen, Position);
                bool horizon = Direction == DrawDirection.Left || Direction == DrawDirection.Right;
                //horizon lines
                int num = horizon ? Lanes : Length;
                float average = Position.Height * 1.0F / num;
                for (int i = 0; i <= num; ++i)
                    graphics.DrawLine(pen, Position.X, Position.Y + average * i, Position.Right, Position.Y + average * i);
                num = horizon ? Length : Lanes;
                average = Position.Width * 1.0F / num;
                for (int i = 0; i <= num; ++i)
                    graphics.DrawLine(pen, Position.X + average * i, Position.Y, Position.X + average * i, Position.Bottom);
            }
            public void DrawRoadId(Graphics graphics)
            {
                var str = Id.ToString();
                var size = graphics.MeasureString(str, Config.Color.Font);
                bool horizon = Direction == DrawDirection.Left || Direction == DrawDirection.Right;
                if (horizon)
                    graphics.DrawString(str, Config.Color.Font, new SolidBrush(Config.Color.Foregroud)
                        , Position.X + Position.Width / 2 - size.Width / 2, Position.Bottom + Config.Size.CarWidth) ;
                else
                    graphics.DrawString(str, Config.Color.Font, new SolidBrush(Config.Color.Foregroud)
                        , Position.Right + Config.Size.CarWidth, Position.Y + Position.Height / 2 - size.Height / 2);
            }
        }

        [System.Runtime.InteropServices.DllImport("gdi32.dll")]
        private static extern bool DeleteObject(IntPtr hObject);
        private static System.Windows.Media.ImageSource ChangeBitmapToImageSource(Bitmap bitmap)
        {
            IntPtr hBitmap = bitmap.GetHbitmap();
            System.Windows.Media.ImageSource wpfBitmap = System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap(
                hBitmap,
                IntPtr.Zero,
                System.Windows.Int32Rect.Empty,
                BitmapSizeOptions.FromEmptyOptions());
            if (!DeleteObject(hBitmap))
                throw new System.ComponentModel.Win32Exception();
            return wpfBitmap;
        }
        private BitmapImage BitmapToBitmapImage(System.Drawing.Bitmap bitmap)
        {
            MemoryStream ms = new MemoryStream();
            bitmap.Save(ms, System.Drawing.Imaging.ImageFormat.Png);
            BitmapImage bit3 = null;
            UIDispatcher.Invoke(new Action(() => {
                bit3 = new BitmapImage();
                bit3.BeginInit();
                bit3.StreamSource = ms;
                bit3.EndInit();
            }));
            Logging.Assert(bit3 != null);
            return bit3;
        }
        private byte[] BitmapToBytes(System.Drawing.Bitmap bitmap)
        {
            MemoryStream ms = new MemoryStream();
            bitmap.Save(ms, System.Drawing.Imaging.ImageFormat.Png);
            ms.Seek(0, SeekOrigin.Begin);
            byte[] bytes = null;
            UIDispatcher.Invoke(new Action(() => {
                using (var binReader = new System.IO.BinaryReader(ms))
                {
                    bytes = binReader.ReadBytes((int)ms.Length);
                }
            }));
            Logging.Assert(bytes != null);
            return bytes;
        }

        private void Drawing()
        {
            if (m_basic == null)
            {
                IsDrawingComplete = true;
                return;
            }

            int index = 0;
            while (!IsIOComplete)
            {
                if (index == m_datas.Count - 1)
                    System.Threading.Thread.Sleep(100);
                for (; index < m_datas.Count; ++index)
                {
                    Logging.Log("draw image : " + index);
                    var chip = m_datas[index];
                    Bitmap image = m_basic.Clone() as Bitmap;
                    using (Graphics graphics = Graphics.FromImage(image))
                    {
                        foreach (var info in chip.Situation)
                        {
                            Road theRoad = m_roads[info.RoadId][info.Forward];
                            for (int lane = 0; lane < info.Cars.Count; ++lane)
                                for (int pos = 0; pos < info.Cars[lane].Count; ++pos)
                                    if (info.Cars[lane][pos] >= 0)
                                        theRoad.DrawCar(graphics, m_cars[info.Cars[lane][pos]], lane, theRoad.Length - pos - 1);
                        }
                    }
                    var file = Config.Path.OutputDir + index + "-" + chip.Time + ".jpg";
                    if (Config.Parameter.SaveImage)
                        image.Save(file);
                    if (Config.Parameter.SaveMemory)
                        m_bytes.Add(new KeyValuePair<int, byte[]>(chip.Time, BitmapToBytes(image)));
                    else
                    //m_images.Add(new KeyValuePair<int, Bitmap>(chip.Time, image));
                        m_images.Add(new KeyValuePair<int, BitmapImage>(chip.Time, BitmapToBitmapImage(image)));
                }
            }
            IsDrawingComplete = true;
        }
    }
}
