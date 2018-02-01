#pragma once

#include "ARIB8CharDecode.h"
#include "AribDescriptor.h"
#include "../../Common/EpgDataCap3Def.h"
#include "../../Common/EpgTimerUtil.h"

class CEpgDBUtil
{
public:
	BOOL AddEIT(WORD PID, const AribDescriptor::CDescriptor& eit, __int64 streamTime);

	BOOL AddServiceListNIT(const AribDescriptor::CDescriptor& nit);
	BOOL AddServiceListSIT(WORD TSID, const AribDescriptor::CDescriptor& sit);
	BOOL AddSDT(const AribDescriptor::CDescriptor& sdt);

	void SetStreamChangeEvent();

	//EPG�f�[�^�̒~�Ϗ�Ԃ����Z�b�g����
	void ClearSectionStatus();

	//�w��T�[�r�X��EPG�f�[�^�̒~�Ϗ�Ԃ��擾����
	//�߂�l�F
	// �X�e�[�^�X
	//�����F
	// originalNetworkID		[IN]�擾�Ώۂ�OriginalNetworkID
	// transportStreamID		[IN]�擾�Ώۂ�TransportStreamID
	// serviceID				[IN]�擾�Ώۂ�ServiceID
	// l_eitFlag				[IN]L-EIT�̃X�e�[�^�X���擾
	EPG_SECTION_STATUS GetSectionStatusService(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		BOOL l_eitFlag
		);

	//EPG�f�[�^�̒~�Ϗ�Ԃ��擾����
	//�߂�l�F
	// �X�e�[�^�X
	//�����F
	// l_eitFlag		[IN]L-EIT�̃X�e�[�^�X���擾
	EPG_SECTION_STATUS GetSectionStatus(BOOL l_eitFlag);

	//�w��T�[�r�X�̑SEPG�����擾����
	//�����F
	// originalNetworkID		[IN]�擾�Ώۂ�originalNetworkID
	// transportStreamID		[IN]�擾�Ώۂ�transportStreamID
	// serviceID				[IN]�擾�Ώۂ�ServiceID
	// epgInfoListSize			[OUT]epgInfoList�̌�
	// epgInfoList				[OUT]EPG���̃��X�g�iDLL���Ŏ����I��delete����B���Ɏ擾���s���܂ŗL���j
	BOOL GetEpgInfoList(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		DWORD* epgInfoListSize,
		EPG_EVENT_INFO** epgInfoList_
		);

	//�w��T�[�r�X�̑SEPG����񋓂���
	BOOL EnumEpgInfoList(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		BOOL (CALLBACK *enumEpgInfoListProc)(DWORD, EPG_EVENT_INFO*, LPVOID),
		LPVOID param
		);

	//�~�ς��ꂽEPG���̂���T�[�r�X�ꗗ���擾����
	//SERVICE_EXT_INFO�̏��͂Ȃ��ꍇ������
	//�����F
	// serviceListSize			[OUT]serviceList�̌�
	// serviceList				[OUT]�T�[�r�X���̃��X�g�iDLL���Ŏ����I��delete����B���Ɏ擾���s���܂ŗL���j
	void GetServiceListEpgDB(
		DWORD* serviceListSize,
		SERVICE_INFO** serviceList_
		);

	//�w��T�[�r�X�̌���or����EPG�����擾����
	//�����F
	// originalNetworkID		[IN]�擾�Ώۂ�originalNetworkID
	// transportStreamID		[IN]�擾�Ώۂ�transportStreamID
	// serviceID				[IN]�擾�Ώۂ�ServiceID
	// nextFlag					[IN]TRUE�i���̔ԑg�j�AFALSE�i���݂̔ԑg�j
	// epgInfo					[OUT]EPG���iDLL���Ŏ����I��delete����B���Ɏ擾���s���܂ŗL���j
	BOOL GetEpgInfo(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		BOOL nextFlag,
		EPG_EVENT_INFO** epgInfo_
		);

	//�w��C�x���g��EPG�����擾����
	//�����F
	// originalNetworkID		[IN]�擾�Ώۂ�originalNetworkID
	// transportStreamID		[IN]�擾�Ώۂ�transportStreamID
	// serviceID				[IN]�擾�Ώۂ�ServiceID
	// EventID					[IN]�擾�Ώۂ�EventID
	// pfOnlyFlag				[IN]p/f����̂݌������邩�ǂ���
	// epgInfo					[OUT]EPG���iDLL���Ŏ����I��delete����B���Ɏ擾���s���܂ŗL���j
	BOOL SearchEpgInfo(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		WORD eventID,
		BYTE pfOnlyFlag,
		EPG_EVENT_INFO** epgInfo_
		);

protected:
	struct SI_TAG{
		BYTE tableID;		//�f�[�^�ǉ�����table_id
		BYTE version;		//�f�[�^�ǉ����̃o�[�W����
		DWORD time;			//�f�[�^�̃^�C���X�^���v(�P�ʂ�10�b)
	};
	struct EVENT_INFO{
		DWORD time;
		SI_TAG tagBasic;
		SI_TAG tagExt;
		EPGDB_EVENT_INFO db;	//ONID,TSID,SID�͖��g�p
	};
	struct SECTION_FLAG_INFO{
		BYTE version;
		BYTE flags[32];			//�Z�O�����g(0�`31)���̎�M�ς݃Z�N�V����(0�`7)�̃t���O
		BYTE ignoreFlags[32];	//��������(���o����Ȃ�)�Z�N�V�����̃t���O
	};
	struct SERVICE_EVENT_INFO{
		map<WORD, EVENT_INFO> eventMap;
		vector<EVENT_INFO> nowEvent;
		vector<EVENT_INFO> nextEvent;
		BYTE lastTableID;
		BYTE lastTableIDExt;
		SECTION_FLAG_INFO sectionList[8];	//�Y�����̓e�[�u���ԍ�(0�`7)
		SECTION_FLAG_INFO sectionExtList[8];
		SERVICE_EVENT_INFO(void){
			lastTableID = 0;
			lastTableIDExt = 0;
		}
	};
	map<ULONGLONG, SERVICE_EVENT_INFO> serviceEventMap;
	map<ULONGLONG, BYTE> serviceList;

	struct DB_TS_INFO{
		wstring network_name;
		wstring ts_name;
		BYTE remote_control_key_id;
		map<WORD, EPGDB_SERVICE_INFO> serviceList;	//network_name,ts_name,remote_control_key_id�͖��g�p
	};
	map<DWORD, DB_TS_INFO> serviceInfoList;

	class CEventInfoStore
	{
	public:
		void Update() {
			data.reset(new EPG_EVENT_INFO[db.size()]);
			adapter.reset(new CEpgEventInfoAdapter[db.size()]);
			for( size_t i = 0; i < db.size(); i++ ){
				data[i] = adapter[i].Create(&db[i]);
			}
		}
		vector<EPGDB_EVENT_INFO> db;
		std::unique_ptr<EPG_EVENT_INFO[]> data;
	private:
		std::unique_ptr<CEpgEventInfoAdapter[]> adapter;
	};
	CEventInfoStore epgInfoList;

	CEventInfoStore epgInfo;

	CEventInfoStore searchEpgInfo;

	std::unique_ptr<SERVICE_INFO[]> serviceDataList;
	std::unique_ptr<EPGDB_SERVICE_INFO[]> serviceDBList;
	std::unique_ptr<CServiceInfoAdapter[]> serviceAdapterList;

	CARIB8CharDecode arib;
protected:
	void Clear();
	
	void AddBasicInfo(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lpParent, WORD onid, WORD tsid);
	void AddShortEvent(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lp);
	BOOL AddExtEvent(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lpParent);
	static void AddContent(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lp);
	void AddComponent(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lp);
	BOOL AddAudioComponent(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lpParent);
	static void AddEventGroup(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lp, WORD onid, WORD tsid);
	static void AddEventRelay(EPGDB_EVENT_INFO* eventInfo, const AribDescriptor::CDescriptor& eit, AribDescriptor::CDescriptor::CLoopPointer lp, WORD onid, WORD tsid);

	static BOOL CheckSectionAll(const SECTION_FLAG_INFO (&sectionList)[8]);
};
