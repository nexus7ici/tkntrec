-- ファイルを転送するスクリプト
-- ファイルをタイムシフト再生できる: http://localhost:5510/xcode.lua?0=video/foo.ts

-- コマンドはEDCBのToolsフォルダにあるものを優先する
ffmpeg=edcb.GetPrivateProfile('SET', 'ModulePath', '', 'Common.ini')..'\\Tools\\ffmpeg.exe'
if not edcb.FindFile(ffmpeg, 1) then ffmpeg='ffmpeg.exe' end
readex=edcb.GetPrivateProfile('SET', 'ModulePath', '', 'Common.ini')..'\\Tools\\readex.exe'
if not edcb.FindFile(readex, 1) then readex='readex.exe' end

-- トランスコードするかどうか(する場合はreadex.exeとffmpeg.exeを用意すること)
XCODE=false
-- 変換コマンド
-- libvpxの例:リアルタイム変換と画質が両立するようにビットレート-bと計算量-cpu-usedを調整する
XCMD='"'..ffmpeg..'" -i pipe:0 -vcodec libvpx -b 896k -quality realtime -cpu-used 1 -vf yadif=0:-1:1 -s 512x288 -r 30000/1001 -acodec libvorbis -ab 128k -f webm -'
-- 変換後の拡張子
XEXT='.webm'
-- 転送開始前に変換しておく量(bytes)
XPREPARE=nil

dofile(mg.script_name:gsub('[^\\/]*$','')..'util.lua')

fpath=nil
for i=0,99 do
  -- 変数名は転送開始位置(99分率)
  fpath=mg.get_var(mg.request_info.query_string, ''..i)
  if fpath then
    fpath=DocumentToNativePath(fpath)
    offset=i
    break
  end
end

f=nil
if fpath then
  fname='xcode'..(fpath:match('%.[0-9A-Za-z]+$') or '')
  fnamets='xcode'..edcb.GetPrivateProfile('SET','TSExt','.ts','EpgTimerSrv.ini'):lower()
  -- 拡張子を限定
  if mg.get_mime_type(fname):find('^video/') or fname:lower()==fnamets then
    f=edcb.io.open(fpath, 'rb')
    if f then
      offset=math.floor((f:seek('end', 0) or 0) * offset / 99 / 188) * 188
      if XCODE then
        f:close()
        -- 容量確保の可能性があるときは周期188+同期語0x47(188*256+0x47=48199)で対象ファイルを終端判定する
        sync=fname:lower()==fnamets and edcb.GetPrivateProfile('SET','KeepDisk',0,'EpgTimerSrv.ini')~='0'
        f=edcb.io.popen('""'..readex..'" '..offset..(sync and ' 4p48199' or ' 4')..' "'..fpath..'" | '..XCMD..'"', 'rb')
        fname='xcode'..XEXT
      else
        -- 容量確保には未対応
        f:seek('set', offset)
        XPREPARE=nil
      end
    end
  end
end

if not f then
  mg.write('HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n')
else
  mg.write('HTTP/1.1 200 OK\r\nContent-Type: '..mg.get_mime_type(fname)..'\r\nContent-Disposition: filename='..fname..'\r\nConnection: close\r\n\r\n')
  retry=0
  while true do
    buf=f:read(XPREPARE or 48128)
    XPREPARE=nil
    if buf and #buf ~= 0 then
      retry=0
      if not mg.write(buf) then
        -- キャンセルされた
        mg.cry('canceled')
        break
      end
    else
      -- 終端に達した。4秒間この状態が続けば対象ファイルへの追記が終息したとみなす
      retry=retry+1
      if XCODE or retry > 20 then
        mg.cry('end')
        break
      end
      edcb.Sleep(200)
    end
  end
  f:close()
end
