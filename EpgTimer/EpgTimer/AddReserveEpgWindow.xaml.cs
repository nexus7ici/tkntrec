﻿using System;
using System.Windows;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Input;
using System.Windows.Threading;

namespace EpgTimer
{
    using EpgView;

    /// <summary>
    /// AddReserveEpgWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class AddReserveEpgWindow : AddReserveEpgWindowBase
    {
        private EpgEventInfo eventInfo = null;
        private bool KeepWin = Settings.Instance.KeepReserveWindow;//固定する
        private bool AddEnabled { get { return eventInfo != null && eventInfo.IsReservable == true; } }
        private bool chgEnabled = false;
        private string tabStr = "予約";

        static AddReserveEpgWindow()
        {
            SearchWindow.ViewReserveUpdated += AddReserveEpgWindow.UpdatesViewSelection;
            EpgViewBase.ViewReserveUpdated += AddReserveEpgWindow.UpdatesViewSelection;
        }
        public AddReserveEpgWindow(EpgEventInfo info = null, int epgInfoOpenMode = 0)
        {
            InitializeComponent();

            base.SetParam(true, checkBox_windowPinned, checkBox_dataReplace);

            //コマンドの登録
            this.CommandBindings.Add(new CommandBinding(EpgCmds.Cancel, (sender, e) => this.Close()));
            this.CommandBindings.Add(new CommandBinding(EpgCmds.AddInDialog, reserve_add, (sender, e) => e.CanExecute = AddEnabled));
            this.CommandBindings.Add(new CommandBinding(EpgCmds.ChangeInDialog, reserve_chg, (sender, e) => e.CanExecute = KeepWin == true && AddEnabled && chgEnabled));
            this.CommandBindings.Add(new CommandBinding(EpgCmds.DeleteInDialog, reserve_del, (sender, e) => e.CanExecute = KeepWin == true && AddEnabled && chgEnabled));
            this.CommandBindings.Add(new CommandBinding(EpgCmds.BackItem, (sender, e) => MoveViewNextItem(-1), (sender, e) => e.CanExecute = KeepWin == true));
            this.CommandBindings.Add(new CommandBinding(EpgCmds.NextItem, (sender, e) => MoveViewNextItem(1), (sender, e) => e.CanExecute = KeepWin == true));
            this.CommandBindings.Add(new CommandBinding(EpgCmds.Search, (sender, e) => MoveViewEpgTarget(), (sender, e) => e.CanExecute = KeepWin == true && DataView is EpgViewBase));

            //ボタンの設定
            mBinds.SetCommandToButton(button_cancel, EpgCmds.Cancel);
            mBinds.SetCommandToButton(button_add_reserve, EpgCmds.AddInDialog);
            mBinds.SetCommandToButton(button_chg_reserve, EpgCmds.ChangeInDialog);
            mBinds.SetCommandToButton(button_del_reserve, EpgCmds.DeleteInDialog);
            mBinds.SetCommandToButton(button_up, EpgCmds.BackItem);
            mBinds.SetCommandToButton(button_down, EpgCmds.NextItem);
            mBinds.SetCommandToButton(button_chk, EpgCmds.Search);
            RefreshMenu();

            //録画設定タブ関係の設定
            recSettingView.SelectedPresetChanged += SetReserveTabHeader;
            reserveTabHeader.MouseRightButtonUp += recSettingView.OpenPresetSelectMenuOnMouseEvent;

            tabControl.SelectedIndex = epgInfoOpenMode == 1 ? 0 : 1;
            if (KeepWin == true)
            {
                button_cancel.Content = "閉じる";
                //ステータスバーの設定
                this.statusBar.Status.Visibility = Visibility.Collapsed;
                StatusManager.RegisterStatusbar(this.statusBar, this);
            }
            else
            {
                button_chg_reserve.Visibility = Visibility.Collapsed;
                button_del_reserve.Visibility = Visibility.Collapsed;
                button_up.Visibility = Visibility.Collapsed;
                button_down.Visibility = Visibility.Collapsed;
                button_chk.Visibility = Visibility.Collapsed;
            }
            ChangeData(info);
        }

        protected override bool ReloadInfoData()
        {
            CheckData(false);
            return true;
        }

        public override void ChangeData(object data)
        {
            var info = data as EpgEventInfo;
            if (data is SearchItem) info = ((SearchItem)data).EventInfo;

            if (info == null) return;

            eventInfo = info;
            Title = ViewUtil.WindowTitleText(eventInfo.DataTitle, "予約登録");
            textBox_info.Text = CommonManager.ConvertProgramText(eventInfo, EventInfoTextMode.BasicOnly);
            richTextBox_descInfo.Document = CommonManager.ConvertDisplayText(eventInfo);
            tabStr = eventInfo.IsOver() == true ? "放映終了" : "予約";

            UpdateViewSelection(0);
            CheckData();
        }
        private void MoveViewEpgTarget()
        {
            if (DataView is EpgViewBase)
            {
                //BeginInvokeはフォーカス対応
                mainWindow.epgView.SearchJumpTargetProgram(eventInfo == null ? 0 : eventInfo.Create64Key());
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    DataView.MoveToProgramItem(eventInfo);
                }), DispatcherPriority.Loaded);
            }
            else
            {
                UpdateViewSelection(3);
            }
        }
        private void CheckData(bool recSetChange = true)
        {
            List<ReserveData> list = GetReserveList();
            chgEnabled = list.Count != 0;
            label_Msg.Visibility = list.Count <= 1 ? Visibility.Hidden : Visibility.Visible;
            button_add_reserve.Content = list.Count == 0 ? "追加" : "重複追加";

            if (chgEnabled == true && recSetChange == true)
            {
                recSettingView.SetDefSetting(list[0].RecSetting);
            }

            SetReserveTabHeader(recSetChange);
        }
        private List<ReserveData> GetReserveList()
        {
            UInt64 id = eventInfo.CurrentPgUID();
            return CommonManager.Instance.DB.ReserveList.Values.Where(data => data.CurrentPgUID() == id).ToList();
        }

        public void SetReserveTabHeader(bool SimpleChanged = true)
        {
            reserveTabHeader.Text = tabStr + recSettingView.GetRecSettingHeaderString(SimpleChanged);
        }

        //proc 0:追加、1:変更、2:削除
        static string[] cmdMsg = new string[] { "追加", "変更", "削除" };
        private void reserve_add(object sender, ExecutedRoutedEventArgs e) { reserve_proc(e, 0); }
        private void reserve_chg(object sender, ExecutedRoutedEventArgs e) { reserve_proc(e, 1); }
        private void reserve_del(object sender, ExecutedRoutedEventArgs e) { reserve_proc(e, 2); }
        private void reserve_proc(ExecutedRoutedEventArgs e, int proc)
        {
            if (CmdExeUtil.IsDisplayKgMessage(e) == true)
            {
                if (MessageBox.Show("予約を" + cmdMsg[proc] + "します。\r\nよろしいですか？", cmdMsg[proc] + "の確認", MessageBoxButton.OKCancel) != MessageBoxResult.OK)
                { return; }
            }

            bool ret = false;

            if (proc == 0)
            {
                ret = MenuUtil.ReserveAdd(CommonUtil.ToList(eventInfo), recSettingView);
            }
            else
            {
                List<ReserveData> list = GetReserveList();
                if (proc == 1)
                {
                    RecSettingData recSet = recSettingView.GetRecSetting();
                    list.ForEach(data => data.RecSetting = recSet);
                    ret = MenuUtil.ReserveChange(list);
                }
                else
                {
                    ret = MenuUtil.ReserveDelete(list);
                }
            }

            StatusManager.StatusNotifySet(ret, "録画予約を" + cmdMsg[proc]);

            if (ret == false) return;
            if (KeepWin == false) this.Close();
        }

        protected override UInt64 DataID { get { return DataView is ReserveView || eventInfo == null ? 0 : eventInfo.CurrentPgUID(); } }
        protected override IEnumerable<KeyValuePair<UInt64, object>> DataRefList
        {
            get
            {
                return CommonManager.Instance.DB.ServiceEventList.Values.SelectMany(list => list.eventMergeList)
                    .Select(d => new KeyValuePair<UInt64, object>(d.CurrentPgUID(), d));
            }
        }
        protected override void UpdateViewSelection(int mode = 0)
        {
            //番組表では「前へ」「次へ」の移動の時だけ追従させる。mode=2はアクティブ時の自動追尾
            var style = JumpItemStyle.MoveTo | (mode < 2 ? JumpItemStyle.PanelNoScroll : JumpItemStyle.None);
            if (DataView is ReserveView)
            {
                if (mode == 2) DataView.MoveToItem(DataID, style);//予約一覧での選択解除
            }
            else if (DataView is EpgMainViewBase)
            {
                if (mode != 2) DataView.MoveToProgramItem(eventInfo, style);
            }
            else if (DataView is EpgListMainView)//mode=0で実行させると重複予約アイテムの選択が解除される。
            {
                if (mode != 0 && mode != 2) DataView.MoveToProgramItem(eventInfo, style);
            }
            else if (DataView is SearchWindow.AutoAddWinListView)
            {
                if (mode != 0) DataView.MoveToItem(DataID, style);//リスト番組表と同様
            }
        }
    }
    public class AddReserveEpgWindowBase : ReserveWindowBase<AddReserveEpgWindow> { }
}
