@echo EpgTimerSrv.exe���T�[�r�X�Ƃ��ăC���X�g�[�����J�n���܂�
@set /p x="���O�I���A�J�E���g��I�� (1=LocalSystem, 2=LocalService) : "
@if "%x%"=="1" goto label1
@if "%x%"=="2" goto label2
@goto label9

:label1
@echo.
@echo LocalSystem�A�J�E���g�ɂ͂ƂĂ���������������܂�
@echo �o�O�̉e�����傫���Ȃ肤�邱�ƁA�������i�̓��ݑ�Ƃ��ė��p�ł��邱�Ƃɒ��ӂ��Ă�������
@pause
cd /d "%~dp0"
sc create "EpgTimer Service" start= auto binPath= "%cd%\EpgTimerSrv.exe"
sc start "EpgTimer Service"
@pause
@goto label9

:label2
@echo.
@echo LocalService�A�J�E���g��EDCB��^��ۑ��t�H���_�ɃA�N�Z�X�ł���悤���ӂ��Ă�������
@pause
cd /d "%~dp0"
sc create "EpgTimer Service" start= auto binPath= "%cd%\EpgTimerSrv.exe" obj= "NT AUTHORITY\LocalService"
sc sidtype "EpgTimer Service" unrestricted
sc start "EpgTimer Service"
@pause
@goto label9

:label9
