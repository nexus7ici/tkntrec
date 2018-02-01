#ifndef INCLUDE_ERR_DEF_H
#define INCLUDE_ERR_DEF_H

#define ERR_FALSE		FALSE		//�ėp�G���[
#define NO_ERR			TRUE		//����
#define ERR_INIT		10			//���������s
#define ERR_NOT_INIT	11			//��������
#define ERR_INVALID_ARG	14			//����������
#define ERR_NOT_FIND	15			//��񂪌�����Ȃ�����

#define CMD_SUCCESS			NO_ERR		//����
#define CMD_ERR				ERR_FALSE	//�ėp�G���[
#define CMD_NEXT			202			//Enum�R�}���h�p�A��������
#define CMD_NON_SUPPORT		203			//���T�|�[�g�̃R�}���h
#define CMD_ERR_INVALID_ARG	204			//�����G���[
#define CMD_ERR_CONNECT		205			//�T�[�o�[�ɃR�l�N�g�ł��Ȃ�����
#define CMD_ERR_DISCONNECT	206			//�T�[�o�[����ؒf���ꂽ
#define CMD_ERR_TIMEOUT		207			//�^�C���A�E�g����
#define CMD_ERR_BUSY		208			//�r�W�[��ԂŌ��ݏ����ł��Ȃ��iEPG�f�[�^�ǂݍ��ݒ��A�^�撆�Ȃǁj
#define CMD_NO_RES			250			//Post�p�Ń��X�|���X�̕K�v�Ȃ�

#endif
