#ifndef INCLUDE_EPG_DATA_CAP3_DEF_H
#define INCLUDE_EPG_DATA_CAP3_DEF_H

//EPG�f�[�^�擾�X�e�[�^�X
typedef enum{
	EpgNoData			= 0x0000,	//�f�[�^���Ȃ�
	EpgNeedData			= 0x0001,	//���܂��Ă��Ȃ�
	EpgBasicAll			= 0x0010,	//��{���͂��܂���
	EpgExtendAll		= 0x0020,	//�g�����͂��܂���
	EpgHEITAll			= 0x0040,	//��{�A�g�����ɂ��܂���
	EpgLEITAll			= 0x0080,	//�����Z�O�����܂���
} EPG_SECTION_STATUS;

//EPG��{���
typedef struct {
	WORD event_nameLength;		//event_name�̕�����
	LPCWSTR event_name;			//�C�x���g��
	WORD text_charLength;		//text_char�̕�����
	LPCWSTR text_char;			//���
} EPG_SHORT_EVENT_INFO;

//EPG�g�����
typedef struct {
	WORD text_charLength;		//text_char�̕�����
	LPCWSTR text_char;			//�ڍ׏��
} EPG_EXTENDED_EVENT_INFO;

//EPG�W�������f�[�^
typedef struct {
	BYTE content_nibble_level_1;
	BYTE content_nibble_level_2;
	BYTE user_nibble_1;
	BYTE user_nibble_2;
}EPG_CONTENT;

//EPG�W���������
typedef struct {
	WORD listSize;
	EPG_CONTENT* nibbleList;
} EPG_CONTEN_INFO;

//EPG�f�����
typedef struct {
	BYTE stream_content;
	BYTE component_type;
	BYTE component_tag;
	WORD text_charLength;		//text_char�̕�����
	LPCWSTR text_char;			//�ڍ׏��
} EPG_COMPONENT_INFO;

//EPG�������
typedef struct {
	BYTE stream_content;
	BYTE component_type;
	BYTE component_tag;
	BYTE stream_type;
	BYTE simulcast_group_tag;
	BYTE ES_multi_lingual_flag;
	BYTE main_component_flag;
	BYTE quality_indicator;
	BYTE sampling_rate;
	WORD text_charLength;		//text_char�̕�����
	LPCWSTR text_char;			//�ڍ׏��
} EPG_AUDIO_COMPONENT_INFO_DATA;

//EPG�������
typedef struct {
	WORD listSize;
	EPG_AUDIO_COMPONENT_INFO_DATA* audioList;
} EPG_AUDIO_COMPONENT_INFO;

//EPG�C�x���g�f�[�^
typedef struct {
	WORD original_network_id;
	WORD transport_stream_id;
	WORD service_id;
	WORD event_id;
}EPG_EVENT_DATA;

//EPG�C�x���g�O���[�v���
typedef struct {
	BYTE group_type;
	BYTE event_count;
	EPG_EVENT_DATA* eventDataList;
} EPG_EVENTGROUP_INFO;

typedef struct {
	WORD event_id;							//�C�x���gID
	BYTE StartTimeFlag;						//start_time�̒l���L�����ǂ���
	SYSTEMTIME start_time;					//�J�n����
	BYTE DurationFlag;						//duration�̒l���L�����ǂ���
	DWORD durationSec;						//�����ԁi�P�ʁF�b�j

	EPG_SHORT_EVENT_INFO* shortInfo;		//��{���
	EPG_EXTENDED_EVENT_INFO* extInfo;		//�g�����
	EPG_CONTEN_INFO* contentInfo;			//�W���������
	EPG_COMPONENT_INFO* componentInfo;		//�f�����
	EPG_AUDIO_COMPONENT_INFO* audioInfo;	//�������
	EPG_EVENTGROUP_INFO* eventGroupInfo;	//�C�x���g�O���[�v���
	EPG_EVENTGROUP_INFO* eventRelayInfo;	//�C�x���g�����[���

	BYTE freeCAFlag;						//�m���X�N�����u���t���O
}EPG_EVENT_INFO;

//�T�[�r�X�̏ڍ׏��
typedef struct {
	BYTE service_type;
	BYTE partialReceptionFlag;
	LPCWSTR service_provider_name;
	LPCWSTR service_name;
	LPCWSTR network_name;
	LPCWSTR ts_name;
	BYTE remote_control_key_id;
}SERVICE_EXT_INFO;

//�T�[�r�X���
typedef struct {
	WORD original_network_id;	//original_network_id
	WORD transport_stream_id;	//transport_stream_id
	WORD service_id;			//service_id
	SERVICE_EXT_INFO* extInfo;	//�ڍ׏��
}SERVICE_INFO;

#endif
