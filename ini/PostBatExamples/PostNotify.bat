@echo off
rem �\����X�V�̃^�C�~���O�ł���ڂ��J�����_�[�ɗ\����A�b�v���[�h����T���v���o�b�`
rem _EDCBX_DIRECT_
rem _EDCBX_#HIDE_ (#����菜���ƃE�B���h�E��\���ɂȂ�)

rem ���ڎ��s���\����X�V�̂Ƃ�����
if not defined NotifyID set NotifyID=2
if "%NotifyID%"=="2" (
  echo ����ڂ��J�����_�[�ɗ\����A�b�v���[�h���܂��B
  powershell -NoProfile -ExecutionPolicy RemoteSigned -File ".\EdcbSchUploader.ps1" �y���[�UID�ƃp�X���[�h�������Ɏw��z

  rem ���퓊�����͂���pause����菜��
  pause
)
