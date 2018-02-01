#ifndef INCLUDE_BON_CTRL_DEF_H
#define INCLUDE_BON_CTRL_DEF_H

#define MUTEX_UDP_PORT_NAME			TEXT("Global\\EpgDataCap_Bon_UDP_PORT_")
#define MUTEX_TCP_PORT_NAME			TEXT("Global\\EpgDataCap_Bon_TCP_PORT_")
#define CHSET_SAVE_EVENT_WAIT		 _T("Global\\EpgTimer_ChSet")


//�l�b�g���[�N���M�p�ݒ�
typedef struct {
	wstring ipString;
	DWORD port;
	BOOL broadcastFlag;
}NW_SEND_INFO;

//EPG�擾�p�T�[�r�X���
typedef struct {
	WORD ONID;
	WORD TSID;
	WORD SID;
}EPGCAP_SERVICE_INFO;


#endif
