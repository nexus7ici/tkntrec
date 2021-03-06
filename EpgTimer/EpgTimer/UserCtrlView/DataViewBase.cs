﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace EpgTimer
{
    public delegate bool ViewUpdatedHandler(DataViewBase sender, bool reloaded);

    public class DataViewBase : UserControl
    {
        public event ViewUpdatedHandler ViewUpdated = null;

        protected static MenuManager mm { get { return CommonManager.Instance.MM; } }
        protected MenuBinds mBinds = new MenuBinds();
        protected string[] status = { "", "", "", "" };
        protected bool ReloadInfoFlg = true;
        protected bool noStatus = false;

        public virtual void UpdateInfo(bool reload = true)
        {
            ReloadInfoFlg |= reload;
            ReloadInfo();
        }
        protected virtual void ReloadInfo()
        {
            if (ReloadInfoFlg == true && this.IsVisible == true)
            {
                ReloadInfoFlg = !ReloadInfoData();
                UpdateStatus();
                if (ViewUpdated != null) ViewUpdated(this, !ReloadInfoFlg);
            }
        }
        protected virtual bool ReloadInfoData() { return true; }
        protected void UpdateStatus(int mode = 0)
        {
            if (noStatus || Settings.Instance.DisplayStatus == false) return;
            UpdateStatusData(mode);
            RefreshStatus();
        }
        protected virtual void UpdateStatusData(int mode = 0) { }
        protected virtual void RefreshStatus(bool force = false)
        {
            if (this.IsVisible == true || force == true)
            {
                StatusManager.StatusSet(status[1], status[2], target: this);
            }
        }
        protected virtual void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            ReloadInfo();
            RefreshStatus();
        }
    }

    //DataListItemBaseのListViewがあるものをベースにする
    public class DataItemViewBase : DataViewBase
    {
        protected override void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            //BlackoutWindow経由のジャンプ
            base.UserControl_IsVisibleChanged(sender, e);
            if (DataListBox != null && this.IsVisible == true)
            {
                if (BlackoutWindow.HasData == true)
                {
                    ItemIdx = ViewUtil.JumpToListItem(BlackoutWindow.SelectedData, DataListBox, BlackoutWindow.NowJumpTable == true ? JumpItemStyle.JumpTo : JumpItemStyle.None);
                }
                BlackoutWindow.Clear();
            }
        }

        //選択アイテムの更新関係
        protected virtual ListBox DataListBox { get { return null; } }
        protected bool IsUnPack { get { return true; } }
        protected object UnPack(object item) { return IsUnPack == false ? item : item is DataListItemBase ? (item as DataListItemBase).DataObj : null; }
        protected int itemIdx = -1;
        protected int ItemIdx { get { return itemIdx; } set { if (value != -1) itemIdx = value; } }
        public virtual void MoveToItem(UInt64 id, JumpItemStyle style = JumpItemStyle.MoveTo)
        {
            if (DataListBox == null || DataListBox.Items.Count == 0) return;

            ItemIdx = ViewUtil.JumpToListItem(id, DataListBox, style);
        }
        public virtual object MoveNextItem(int direction, UInt64 id = 0, bool move = true, JumpItemStyle style = JumpItemStyle.MoveTo)
        {
            if (DataListBox == null || DataListBox.Items.Count == 0) return null;

            var list = DataListBox.Items.OfType<IGridViewSorterItem>().ToList();
            var selected = DataListBox.SelectedIndex == -1 ? list.FindIndex(d => d.KeyID == id) : DataListBox.SelectedIndex;
            var item = list[ViewUtil.GetNextIdx(itemIdx, selected, list.Count, direction)];
            if (move == true) ItemIdx = ViewUtil.ScrollToFindItem(item, DataListBox, style);
            return UnPack(item);
        }

        //SearchItem関係
        public virtual void MoveToReserveItem(ReserveData target, JumpItemStyle style = JumpItemStyle.MoveTo)
        {
            if (DataListBox == null || DataListBox.Items.Count == 0) return;

            if (target != null && target.IsEpgReserve == true)
            {
                //重複予約が無ければ、target.CurrentPgUID()でMoveToItem()に投げられる。
                var item = DataListBox.Items.OfType<SearchItem>().FirstOrDefault(d => d.IsReserved == true && d.ReserveInfo.ReserveID == target.ReserveID);
                ItemIdx = ViewUtil.ScrollToFindItem(item, DataListBox, style);
            }
            else
            {
                //プログラム予約だと見つからないので、それらしい番組へジャンプする。
                MoveToProgramItem(target == null ? null : target.SearchEventInfoLikeThat(), style);
            }
        }
        public virtual void MoveToProgramItem(EpgEventInfo target, JumpItemStyle style = JumpItemStyle.MoveTo)
        {
            if (DataListBox == null || DataListBox.Items.Count == 0) return;

            //過去番組表でイベントIDが重複している場合があるので開始時間も考慮する
            MoveToItem(target == null ? 0 : target.CurrentPgUID(), style);
        }

        //予約データの移動関係、SearchWindowとEpgListMainView
        public virtual object MoveNextReserve(int direction, UInt64 id = 0, bool move = true, JumpItemStyle style = JumpItemStyle.MoveTo)
        {
            if (DataListBox == null || DataListBox.Items.Count == 0) return null;

            var list = DataListBox.Items.OfType<SearchItem>().ToList();
            var idx = id == 0 ? -1 : list.FindIndex(d => d.IsReserved == true && d.ReserveInfo.ReserveID == id);
            idx = idx != -1 ? idx : DataListBox.SelectedIndex != -1 ? DataListBox.SelectedIndex : itemIdx;
            idx++;

            List<SearchItem> sList = list.Skip(idx).Concat(list.Take(idx)).ToList();
            if (direction < 0) sList.Reverse(0, sList.Count - (idx == 0 ? 0 : 1));
            SearchItem item = sList.FirstOrDefault(info => info.IsReserved == true);

            if (move == true) ItemIdx = ViewUtil.ScrollToFindItem(item, DataListBox, style);
            return item == null ? null : item.ReserveInfo;
        }
    }
}
