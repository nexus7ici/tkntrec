﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace EpgTimer
{
    using Setting;

    /// <summary>
    /// SettingWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class SettingWindow : AttendantWindow
    {
        private HashSet<string> msgSet = new HashSet<string>();

        public static void UpdatesInfo(string msg = null)
        {
            foreach (var win in Application.Current.Windows.OfType<SettingWindow>())
            {
                win.SetReload(true, msg);
            }
        }
        private void SetReload(bool reload, string msg = null)
        {
            if (string.IsNullOrWhiteSpace(msg) == false) msgSet.Add(msg);
            if (reload == false) msgSet.Clear();
            button_Reload.Content = "再読込" + (reload == false ? "" : "*");
            button_Reload.ToolTip = reload == false ? null :
                ("他の操作により設定が変更されています" + (msgSet.Count == 0 ? null : "\r\n *" + string.Join("\r\n *", msgSet)));
        }

        public enum SettingMode { Default, EpgSetting }
        public SettingMode Mode { get; private set; }

        public SettingWindow(SettingMode mode = SettingMode.Default, object param = null)
        {
            InitializeComponent();

            //設定ウィンドウについては最低サイズを決めておく。
            if (Height < 580) Height = 580;
            if (Width < 780) Width = 780;

            base.SetParam(false, new CheckBox());
            this.Pinned = true;

            button_Reload.Click += (sender, e) => LoadSetting();
            button_Apply.Click += (sender, e) => { Apply(); LoadSetting(); };
            button_OK.Click += (sender, e) => { this.Close(); Apply(); };
            button_cancel.Click += (sender, e) => this.Close();

            LoadSetting();
            SetMode(mode, param);
        }

        public void LoadSetting()
        {
            try
            {
                DataContext = Settings.Instance.DeepCloneStaticSettings();
                setBasicView.LoadSetting();
                setAppView.LoadSetting();
                setEpgView.LoadSetting();
                setOtherAppView.LoadSetting();
                SetReload(false);
            }
            catch (Exception ex) { MessageBox.Show(ex.Message + "\r\n" + ex.StackTrace); }
        }

        public void SetMode(SettingMode mode, object param)
        {
            Mode = mode;
            switch (mode)
            {
                case SettingMode.EpgSetting:
                    tabItem_epgView.IsSelected = true;
                    setEpgView.tabEpg.IsSelected = true;
                    setEpgView.tabEpgTab.IsSelected = true;
                    setEpgView.listBox_tab.SelectedItem = setEpgView.listBox_tab.Items.OfType<CustomEpgTabInfoView>().FirstOrDefault(item => item.Info.Uid == param as string);
                    break;
            }
        }

        private void Apply()
        {
            try
            {
                setBasicView.SaveSetting();
                setAppView.SaveSetting();
                setEpgView.SaveSetting();
                setOtherAppView.SaveSetting();

                Settings.Instance.ShallowCopyDynamicSettingsTo((Settings)DataContext);
                Settings.Instance = (Settings)DataContext;
                SettingWindow.UpdatesInfo("別画面/PCでの設定更新");//基本的に一つしか使わないが一応通知

                if (CommonManager.Instance.NWMode == false)
                {
                    ChSet5.SaveFile();
                }
                CommonManager.Instance.ReloadCustContentColorList();
                CommonManager.ReloadReplaceDictionary();
                ItemFontCache.Clear();

                ViewUtil.MainWindow.SaveData();
                ViewUtil.MainWindow.RefreshSetting(this);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message + "\r\n" + ex.StackTrace);
                MessageBox.Show("不正な入力値によるエラーのため、一部設定のみ更新されました。");
            }
        }
    }
}
