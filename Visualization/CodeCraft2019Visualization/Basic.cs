using System.Collections.Generic;
using System.Drawing;

namespace CodeCraft2019Visualization
{
    partial class Scenario
    {
        partial class Road
        {
            public enum DrawDirection
            {
                Up,
                Down,
                Left,
                Right
            }
            public int Id { get; set; } = 0;
            public int Lanes { get; set; } = 0;
            public int Length { get; set; } = 0;
            public Cross From { get; set; } = null;
            public Cross To { get; set; } = null;
        }

        partial class Cross
        {
            public int Id { get; set; } = 0;
            public int[] Neighbors { get; } = new int[] { -1, -1, -1, -1 };
        }

        class Car
        {
            public enum CarType
            {
                Normal,
                Preset,
                Force
            }
            public int Id { get; set; } = 0;
            public CarType Type { get; set; } = CarType.Normal;
            public bool IsVip { get; set; } = false;
        }

        class TimeChip
        {
            public class ChipInfo
            {
                public int RoadId { get; set; } = 0;
                public bool Forward { get; set; } = true;
                //lane -> position
                public List<List<int>> Cars { get; set; } = new List<List<int>>();
            }
            public int Time { get; set; } = 0;
            public List<ChipInfo> Situation { get; set; } = new List<ChipInfo>();
        }
    }
}
