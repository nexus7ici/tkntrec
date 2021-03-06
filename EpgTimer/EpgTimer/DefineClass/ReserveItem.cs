﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows;

namespace EpgTimer
{
    public class ReserveItem : SearchItem
    {
        public ReserveItem() { }
        public ReserveItem(ReserveData item) { base.ReserveInfo = item; }

        public override ulong KeyID { get { return ReserveInfo == null ? 0 : ReserveInfo.ReserveID; } }
        public override object DataObj { get { return ReserveInfo; } }

        public override EpgEventInfo EventInfo
        {
            get
            {
                if (eventInfo == null)
                {
                    if (ReserveInfo != null)
                    {
                        eventInfo = ReserveInfo.SearchEventInfo(false);
                    }
                }
                return eventInfo;
            }
        }

        public override String EventName
        {
            get
            {
                if (ReserveInfo == null) return "";
                //
                return ReserveInfo.Title;
            }
        }
        public override String ServiceName
        {
            get
            {
                if (ReserveInfo == null) return "";
                //
                return ReserveInfo.StationName;
            }
        }
        public override String NetworkName
        {
            get
            {
                if (ReserveInfo == null) return "";
                //
                return CommonManager.ConvertNetworkNameText(ReserveInfo.OriginalNetworkID);
            }
        }
        public override String StartTime
        {
            get
            {
                if (ReserveInfo == null) return "";
                //
                return GetTimeStringReserveStyle(ReserveInfo.StartTime, ReserveInfo.DurationSecond);
            }
        }
        public static String GetTimeStringReserveStyle(DateTime time, uint durationSecond)
        {
            return CommonManager.ConvertTimeText(time, durationSecond, Settings.Instance.ResInfoNoYear, Settings.Instance.ResInfoNoSecond);
        }
        public override long StartTimeValue
        {
            get
            {
                if (ReserveInfo == null) return long.MinValue;
                //
                return ReserveInfo.StartTime.Ticks;
            }
        }
        public String StartTimeShort
        {
            get
            {
                if (ReserveInfo == null) return "";
                //
                return CommonManager.ConvertTimeText(ReserveInfo.StartTime, ReserveInfo.DurationSecond, true, true);
            }
        }
        public override String ProgramDuration
        {
            get
            {
                if (ReserveInfo == null) return "";
                //
                return GetDurationStringReserveStyle(ReserveInfo.DurationSecond);
            }
        }
        public static String GetDurationStringReserveStyle(uint durationSecond)
        {
            return CommonManager.ConvertDurationText(durationSecond, Settings.Instance.ResInfoNoDurSecond);
        }
        public override UInt32 ProgramDurationValue
        {
            get
            {
                if (ReserveInfo == null) return UInt32.MinValue;
                //
                return ReserveInfo.DurationSecond;
            }
        }
        public override String ConvertInfoText(object param = null)
        {
            var mode = param is Int32 ? (Int32)param : Settings.Instance.ReserveToolTipMode;
            if (mode == 1) return base.ConvertInfoText();

            if (ReserveInfo == null) return "";
            //
            String view = CommonManager.ConvertTimeText(ReserveInfo.StartTime, ReserveInfo.DurationSecond, false, false, false) + "\r\n";
            view += ServiceName + "(" + NetworkName + ")" + "\r\n";
            view += EventName + "\r\n\r\n";

            view += ConvertRecSettingText() + "\r\n";
            view += "予約状況 : " + Comment + "\r\n\r\n";

            view += CommonManager.Convert64PGKeyString(ReserveInfo.Create64PgKey());

            return view;
        }

        static String[] wiewString = { "", "", "無", "予+", "予+", "無+", "録*", "視*", "無*" };
        public override String Status
        {
            get
            {
                int index = 0;
                if (ReserveInfo != null)
                {
                    if (ReserveInfo.IsOnAir() == true)
                    {
                        index = 3;
                    }
                    if (ReserveInfo.IsOnRec() == true)//マージンがあるので、IsOnAir==trueとは限らない
                    {
                        index = 6;
                    }
                    if (ReserveInfo.IsEnabled == false) //無効の判定
                    {
                        index += 2;
                    }
                    else if (ReserveInfo.IsWatchMode == true) //視聴中の判定
                    {
                        index += 1;
                    }
                }
                return wiewString[index];
            }
        }
        public override Brush StatusColor
        {
            get
            {
                if (ReserveInfo != null)
                {
                    if (ReserveInfo.IsOnRec() == true)
                    {
                        return CommonManager.Instance.ResStatusColor[ReserveInfo.IsWatchMode ? 3 : 1];
                    }
                    if (ReserveInfo.IsOnAir() == true)
                    {
                        return CommonManager.Instance.ResStatusColor[2];
                    }
                }
                return CommonManager.Instance.ResStatusColor[0];
            }
        }
    }
}
