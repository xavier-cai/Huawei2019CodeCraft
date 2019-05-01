using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace CodeCraft2019Visualization
{
    partial class Scenario
    {
        private static Regex IORegex = new Regex(@"\((\d+),(forward|backward),\[(.+)\]\)", RegexOptions.Compiled);
        private static Regex IOLaneRegex = new Regex(@"\[(.+?)\]");
        private void IO()
        {
            Logging.Log("read file : " + Config.Path.LogFullPath);
            using (var fs = new FileStream(Config.Path.LogFullPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
            {
                if (fs == null) return;
                TimeChip chip = null;
                using (var sr = new StreamReader(fs))
                {
                    int line = 0;
                    while (!sr.EndOfStream)
                    {
                        ++line;
                        var str = sr.ReadLine();
                        if (str.Substring(0, 5).Contains("time:"))
                        {
                            if (chip != null)
                            {
                                m_datas.Add(chip);
                            }
                            chip = new TimeChip();
                            try
                            {
                                chip.Time = Convert.ToInt32(str.Substring(5));
                                Logging.Log("read time chip : " + m_datas.Count + " time is " + chip.Time);
                            }
                            catch
                            {
                                Logging.Log("unexpected time line [" + line + "] : " + str);
                                break;
                            }
                        }
                        else if (str[0] == '(')
                        {
                            Logging.Assert(chip != null);
                            var match = IORegex.Match(str).Groups;
                            if (match.Count != 4)
                            {
                                Logging.Log("unexpected line [" + line + "] : " + str);
                            }
                            else
                            {
                                TimeChip.ChipInfo info = new TimeChip.ChipInfo();
                                info.RoadId = Convert.ToInt32(match[1].Value);
                                var dir = match[2].Value;
                                Logging.Assert(dir == "forward" || dir == "backward");
                                info.Forward = dir == "forward";
                                var matches = IOLaneRegex.Matches(match[3].Value);
                                foreach (var eMatch in matches)
                                {
                                    Logging.Assert((eMatch as Match).Groups.Count == 2);
                                    info.Cars.Add((eMatch as Match).Groups[1].Value.Split(',').Aggregate(new List<int>(), (l, s) => { l.Add(Convert.ToInt32(s)); return l; }));
                                }
                                chip.Situation.Add(info);
                            }
                        }
                        else
                        {
                            Logging.Log("unexpected line [" + line + "] : " + str);
                        }
                    }
                }
            }
            IsIOComplete = true;
        }
    }
}
