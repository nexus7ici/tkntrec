#include "stdafx.h"
#include "ParseTextInstances.h"
#include "TimeUtil.h"
#include "PathUtil.h"

//�^�u��؂�̎��̃g�[�N���Ɉړ�����
static LPCWSTR NextToken(LPCWSTR* token)
{
	//token�ɂ͌��݂̃g�[�N���擪/����/���̃g�[�N���擪���i�[����
	token[0] = token[2];
	for( ; *token[2] != L'\0'; token[2]++ ){
		if( *token[2] == L'\t' ){
			token[1] = token[2]++;
			return token[0];
		}
	}
	token[1] = token[2];
	return token[0];
}

//���s���^�u�ɁA�^�u���󔒂ɒu�����āA�c�����^�u�̐���Ԃ�
static DWORD FinalizeField(wstring& str)
{
	DWORD tabCount = 0;
	for( size_t i = 0; i < str.size(); i++ ){
		if( str[i] == L'\n' ){
			str[i] = L'\t';
			tabCount++;
		}else if( str[i] == L'\t' ){
			str[i] = L' ';
		}
	}
	return tabCount;
}

vector<CH_DATA4*> CParseChText4::GetChDataList()
{
	vector<CH_DATA4*> chDataList;
	for( map<DWORD, CH_DATA4>::iterator itr = this->itemMap.begin(); itr != this->itemMap.end(); itr++ ){
		chDataList.push_back(&itr->second);
	}
	return chDataList;
}

DWORD CParseChText4::AddCh(const CH_DATA4& item)
{
	map<DWORD, CH_DATA4>::const_iterator itr =
		this->itemMap.insert(pair<DWORD, CH_DATA4>(this->itemMap.empty() ? 1 : this->itemMap.rbegin()->first + 1, item)).first;
	return itr->first;
}

void CParseChText4::DelChService(int space, int ch, WORD serviceID)
{
	map<DWORD, CH_DATA4>::const_iterator itr = this->itemMap.begin();
	while( itr != this->itemMap.end() ){
		if( itr->second.space == space && itr->second.ch == ch && itr->second.serviceID == serviceID ){
			itr = this->itemMap.erase(itr);
		}else{
			itr++;
		}
	}
}

bool CParseChText4::ParseLine(LPCWSTR parseLine, pair<DWORD, CH_DATA4>& item)
{
	if( wcschr(parseLine, L'\t') == NULL ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	NextToken(token);
	item.second.chName.assign(token[0], token[1]);
	NextToken(token);
	item.second.serviceName.assign(token[0], token[1]);
	NextToken(token);
	item.second.networkName.assign(token[0], token[1]);

	item.second.space = _wtoi(NextToken(token));
	item.second.ch = _wtoi(NextToken(token));
	item.second.originalNetworkID = (WORD)_wtoi(NextToken(token));
	item.second.transportStreamID = (WORD)_wtoi(NextToken(token));
	item.second.serviceID = (WORD)_wtoi(NextToken(token));
	item.second.serviceType = (WORD)_wtoi(NextToken(token));
	item.second.partialFlag = _wtoi(NextToken(token)) != 0;
	item.second.useViewFlag = _wtoi(NextToken(token)) != 0;
	item.second.remoconID = (BYTE)_wtoi(NextToken(token));
	item.first = this->itemMap.empty() ? 1 : this->itemMap.rbegin()->first + 1;
	return true;
}

bool CParseChText4::SaveLine(const pair<DWORD, CH_DATA4>& item, wstring& saveLine) const
{
	Format(saveLine, L"%s\n%s\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d",
		item.second.chName.c_str(),
		item.second.serviceName.c_str(),
		item.second.networkName.c_str(),
		item.second.space,
		item.second.ch,
		item.second.originalNetworkID,
		item.second.transportStreamID,
		item.second.serviceID,
		item.second.serviceType,
		item.second.partialFlag,
		item.second.useViewFlag,
		item.second.remoconID
		);
	return FinalizeField(saveLine) == 11;
}

LONGLONG CParseChText5::AddCh(const CH_DATA5& item)
{
	LONGLONG key = (LONGLONG)item.originalNetworkID << 32 | (LONGLONG)item.transportStreamID << 16 | item.serviceID;
	this->itemMap[key] = item;
	return key;
}

bool CParseChText5::SetEpgCapMode(WORD originalNetworkID, WORD transportStreamID, WORD serviceID, BOOL epgCapFlag)
{
	map<LONGLONG, CH_DATA5>::iterator itr = this->itemMap.find((LONGLONG)originalNetworkID << 32 | (LONGLONG)transportStreamID << 16 | serviceID);
	if( itr != this->itemMap.end() ){
		itr->second.epgCapFlag = epgCapFlag;
		return true;
	}
	return false;
}

bool CParseChText5::ParseLine(LPCWSTR parseLine, pair<LONGLONG, CH_DATA5>& item)
{
	if( wcschr(parseLine, L'\t') == NULL ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	NextToken(token);
	item.second.serviceName.assign(token[0], token[1]);
	NextToken(token);
	item.second.networkName.assign(token[0], token[1]);

	item.second.originalNetworkID = (WORD)_wtoi(NextToken(token));
	item.second.transportStreamID = (WORD)_wtoi(NextToken(token));
	item.second.serviceID = (WORD)_wtoi(NextToken(token));
	item.second.serviceType = (WORD)_wtoi(NextToken(token));
	item.second.partialFlag = _wtoi(NextToken(token)) != 0;
	item.second.epgCapFlag = _wtoi(NextToken(token)) != 0;
	item.second.searchFlag = _wtoi(NextToken(token)) != 0;
	item.first = (LONGLONG)item.second.originalNetworkID << 32 | (LONGLONG)item.second.transportStreamID << 16 | item.second.serviceID;
	return true;
}

bool CParseChText5::SaveLine(const pair<LONGLONG, CH_DATA5>& item, wstring& saveLine) const
{
	Format(saveLine, L"%s\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d",
		item.second.serviceName.c_str(),
		item.second.networkName.c_str(),
		item.second.originalNetworkID,
		item.second.transportStreamID,
		item.second.serviceID,
		item.second.serviceType,
		item.second.partialFlag,
		item.second.epgCapFlag,
		item.second.searchFlag
		);
	return FinalizeField(saveLine) == 8;
}

void CParseContentTypeText::GetMimeType(wstring ext, wstring& mimeType) const
{
	map<wstring, wstring>::const_iterator itr = this->itemMap.find(ext);
	if( itr == this->itemMap.end() ){
		mimeType = L"application/octet-stream";
	}else{
		mimeType = itr->second;
	}
}

bool CParseContentTypeText::ParseLine(LPCWSTR parseLine, pair<wstring, wstring>& item)
{
	if( wcschr(parseLine, L'\t') == NULL || parseLine[0] == L';' ){
		//��sor�R�����g
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	NextToken(token);
	item.first.assign(token[0], token[1]);
	NextToken(token);
	item.second.assign(token[0], token[1]);
	return true;
}

void CParseServiceChgText::ChgText(wstring& chgText) const
{
	map<wstring, wstring>::const_iterator itr = this->itemMap.find(chgText);
	if( itr != this->itemMap.end() ){
		chgText = itr->second;
	}
}

bool CParseServiceChgText::ParseLine(LPCWSTR parseLine, pair<wstring, wstring>& item)
{
	if( wcschr(parseLine, L'\t') == NULL || parseLine[0] == L';' ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	NextToken(token);
	item.first.assign(token[0], token[1]);
	NextToken(token);
	item.second.assign(token[0], token[1]);
	return true;
}

DWORD CParseRecInfoText::AddRecInfo(const REC_FILE_INFO& item)
{
	REC_FILE_INFO info = item;
	info.id = this->nextID++;
	this->itemMap[info.id] = info;

	//��v���e�N�g�̗v�f����keepCount�ȉ��ɂȂ�܂ō폜
	DWORD protectCount = 0;
	map<DWORD, REC_FILE_INFO>::const_iterator itr;
	for( itr = this->itemMap.begin(); itr != this->itemMap.end(); itr++ ){
		if( itr->second.protectFlag != 0 ){
			protectCount++;
		}
	}
	for( itr = this->itemMap.begin(); itr != this->itemMap.end() && this->itemMap.size() - protectCount > this->keepCount; ){
		if( itr->second.protectFlag == 0 ){
			OnDelRecInfo(itr->second);
			itr = this->itemMap.erase(itr);
		}else{
			itr++;
		}
	}
	return info.id;
}

bool CParseRecInfoText::DelRecInfo(DWORD id)
{
	map<DWORD, REC_FILE_INFO>::const_iterator itr = this->itemMap.find(id);
	if( itr != this->itemMap.end() && itr->second.protectFlag == 0 ){
		OnDelRecInfo(itr->second);
		this->itemMap.erase(itr);
		return true;
	}
	return false;
}

bool CParseRecInfoText::ChgProtectRecInfo(DWORD id, BYTE flag)
{
	map<DWORD, REC_FILE_INFO>::iterator itr = this->itemMap.find(id);
	if( itr != this->itemMap.end() ){
		itr->second.protectFlag = flag;
		return true;
	}
	return false;
}

void CParseRecInfoText::SetRecInfoFolder(LPCWSTR folder)
{
	this->recInfoFolder = folder;
}

bool CParseRecInfoText::ParseLine(LPCWSTR parseLine, pair<DWORD, REC_FILE_INFO>& item)
{
	if( wcschr(parseLine, L'\t') == NULL || parseLine[0] == L';' ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	NextToken(token);
	item.second.recFilePath.assign(token[0], token[1]);
	NextToken(token);
	item.second.title.assign(token[0], token[1]);

	FILETIME ft;
	item.second.startTime.wMilliseconds = 0;
	if( swscanf_s(NextToken(token), L"%hu/%hu/%hu", &item.second.startTime.wYear, &item.second.startTime.wMonth, &item.second.startTime.wDay) != 3 ||
	    swscanf_s(NextToken(token), L"%hu:%hu:%hu", &item.second.startTime.wHour, &item.second.startTime.wMinute, &item.second.startTime.wSecond) != 3 ||
	    SystemTimeToFileTime(&item.second.startTime, &ft) == FALSE ||
	    FileTimeToSystemTime(&ft, &item.second.startTime) == FALSE ){
		return false;
	}
	WORD wDuration[3];
	if( swscanf_s(NextToken(token), L"%hu:%hu:%hu", &wDuration[0], &wDuration[1], &wDuration[2]) != 3 ){
		return false;
	}
	item.second.durationSecond = (wDuration[0] * 60 + wDuration[1]) * 60 + wDuration[2];

	NextToken(token);
	item.second.serviceName.assign(token[0], token[1]);
	item.second.originalNetworkID = (WORD)_wtoi(NextToken(token));
	item.second.transportStreamID = (WORD)_wtoi(NextToken(token));
	item.second.serviceID = (WORD)_wtoi(NextToken(token));
	item.second.eventID = (WORD)_wtoi(NextToken(token));
	item.second.drops = _wtoi64(NextToken(token));
	item.second.scrambles = _wtoi64(NextToken(token));
	item.second.recStatus = _wtoi(NextToken(token));

	item.second.startTimeEpg.wMilliseconds = 0;
	if( swscanf_s(NextToken(token), L"%hu/%hu/%hu", &item.second.startTimeEpg.wYear, &item.second.startTimeEpg.wMonth, &item.second.startTimeEpg.wDay) != 3 ||
	    swscanf_s(NextToken(token), L"%hu:%hu:%hu", &item.second.startTimeEpg.wHour, &item.second.startTimeEpg.wMinute, &item.second.startTimeEpg.wSecond) != 3 ||
	    SystemTimeToFileTime(&item.second.startTimeEpg, &ft) == FALSE ||
	    FileTimeToSystemTime(&ft, &item.second.startTimeEpg) == FALSE ){
		return false;
	}
	NextToken(token);
	item.second.protectFlag = _wtoi(NextToken(token)) != 0;
	item.second.id = this->nextID++;
	item.first = item.second.id;
	return true;
}

bool CParseRecInfoText::SaveLine(const pair<DWORD, REC_FILE_INFO>& item, wstring& saveLine) const
{
	Format(saveLine, L"%s\n%s\n%04d/%02d/%02d\n%02d:%02d:%02d\n%02d:%02d:%02d\n%s\n%d\n%d\n%d\n%d\n%I64d\n%I64d\n%d\n%04d/%02d/%02d\n%02d:%02d:%02d\n%s\n%d\n",
		item.second.recFilePath.c_str(),
		item.second.title.c_str(),
		item.second.startTime.wYear, item.second.startTime.wMonth, item.second.startTime.wDay,
		item.second.startTime.wHour, item.second.startTime.wMinute, item.second.startTime.wSecond,
		item.second.durationSecond / 60 / 60, item.second.durationSecond / 60 % 60, item.second.durationSecond % 60,
		item.second.serviceName.c_str(),
		item.second.originalNetworkID,
		item.second.transportStreamID,
		item.second.serviceID,
		item.second.eventID,
		item.second.drops,
		item.second.scrambles,
		item.second.recStatus,
		item.second.startTimeEpg.wYear, item.second.startTimeEpg.wMonth, item.second.startTimeEpg.wDay,
		item.second.startTimeEpg.wHour, item.second.startTimeEpg.wMinute, item.second.startTimeEpg.wSecond,
		item.second.GetComment(),
		item.second.protectFlag
		);
	return FinalizeField(saveLine) == 17;
}

bool CParseRecInfoText::SelectIDToSave(vector<DWORD>& sortList) const
{
	vector<const REC_FILE_INFO*> work;
	work.reserve(this->itemMap.size());
	for( map<DWORD, REC_FILE_INFO>::const_iterator itr = this->itemMap.begin(); itr != this->itemMap.end(); itr++ ){
		work.push_back(&itr->second);
	}
	std::sort(work.begin(), work.end(), [](const REC_FILE_INFO* a, const REC_FILE_INFO* b) -> bool {
		const SYSTEMTIME& sa = a->startTime;
		const SYSTEMTIME& sb = b->startTime;
		return sa.wYear < sb.wYear || sa.wYear == sb.wYear && (
		       sa.wMonth < sb.wMonth || sa.wMonth == sb.wMonth && (
		       sa.wDay < sb.wDay || sa.wDay == sb.wDay && (
		       sa.wHour < sb.wHour || sa.wHour == sb.wHour && (
		       sa.wMinute < sb.wMinute || sa.wMinute == sb.wMinute && (
		       sa.wSecond < sb.wSecond || sa.wSecond == sb.wSecond && (
		       a->originalNetworkID < b->originalNetworkID || a->originalNetworkID == b->originalNetworkID && (
		       a->transportStreamID < b->transportStreamID)))))));
	});
	sortList.reserve(work.size());
	for( size_t i = 0; i < work.size(); i++ ){
		sortList.push_back(work[i]->id);
	}
	return true;
}

wstring CParseRecInfoText::GetExtraInfo(LPCWSTR recFilePath, LPCWSTR extension, const wstring& resultOfGetRecInfoFolder, bool recInfoFolderOnly)
{
	wstring info;
	if( recFilePath[0] != L'\0' ){
		//�⑫�̘^����t�@�C����ǂݍ���
		DWORD shareAll = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		if( resultOfGetRecInfoFolder.empty() || recInfoFolderOnly == false ){
			hFile = CreateFile((wstring(recFilePath) + extension).c_str(), GENERIC_READ, shareAll, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		if( hFile == INVALID_HANDLE_VALUE && resultOfGetRecInfoFolder.empty() == false ){
			fs_path infoPath = fs_path(resultOfGetRecInfoFolder).append(fs_path(recFilePath).filename().concat(extension).native());
			hFile = CreateFile(infoPath.c_str(), GENERIC_READ, shareAll, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		if( hFile != INVALID_HANDLE_VALUE ){
			DWORD dwSize = GetFileSize(hFile, NULL);
			if( dwSize != 0 && dwSize != INVALID_FILE_SIZE ){
				vector<char> buf(dwSize);
				DWORD dwRead;
				if( ReadFile(hFile, buf.data(), dwSize, &dwRead, NULL) ){
					if( dwRead >= 3 && buf[0] == '\xEF' && buf[1] == '\xBB' && buf[2] == '\xBF' ){
						UTF8toW(string(buf.begin() + 3, buf.begin() + dwRead), info);
					}else{
						AtoW(string(buf.begin(), buf.begin() + dwRead), info);
					}
				}
			}
			CloseHandle(hFile);
		}
	}
	return info;
}

void CParseRecInfoText::OnDelRecInfo(const REC_FILE_INFO& item)
{
	if( item.recFilePath.empty() || this->recInfoDelFile == false ){
		return;
	}
	//�^��t�@�C�����폜����
	DeleteFile(item.recFilePath.c_str());
	if( this->customizeDelExt ){
		//�J�X�^�����[��
		OutputDebugString((L"��RecInfo Auto Delete : " + item.recFilePath + L"\r\n").c_str());
		wstring debug;
		for( size_t i = 0; i < this->customDelExt.size(); i++ ){
			wstring delPath = fs_path(item.recFilePath).replace_extension().native();
			DeleteFile((delPath + this->customDelExt[i]).c_str());
			debug = (debug.empty() ? delPath + L'(' : debug + L'|') + this->customDelExt[i];
		}
		if( debug.empty() == false ){
			OutputDebugString((L"��RecInfo Auto Delete : " + debug + L")\r\n").c_str());
		}
		if( this->recInfoFolder.empty() == false ){
			//�^����t�H���_�ɂ��K�p
			debug.clear();
			for( size_t i = 0; i < this->customDelExt.size(); i++ ){
				wstring delPath = fs_path(this->recInfoFolder).append(fs_path(item.recFilePath).stem().native()).native();
				DeleteFile((delPath + this->customDelExt[i]).c_str());
				debug = (debug.empty() ? delPath + L'(' : debug + L'|') + this->customDelExt[i];
			}
			if( debug.empty() == false ){
				OutputDebugString((L"��RecInfo Auto Delete : " + debug + L")\r\n").c_str());
			}
		}
	}else{
		//�W���̃��[��
		DeleteFile((item.recFilePath + L".err").c_str());
		DeleteFile((item.recFilePath + L".program.txt").c_str());
		OutputDebugString((L"��RecInfo Auto Delete : " + item.recFilePath + L"(.err|.program.txt)\r\n").c_str());
		if( this->recInfoFolder.empty() == false ){
			//�^����t�H���_�ɂ��K�p
			wstring delPath = fs_path(this->recInfoFolder).append(fs_path(item.recFilePath).filename().native()).native();
			DeleteFile((delPath + L".err").c_str());
			DeleteFile((delPath + L".program.txt").c_str());
			OutputDebugString((L"��RecInfo Auto Delete : " + delPath + L"(.err|.program.txt)\r\n").c_str());
		}
	}
}

DWORD CParseRecInfo2Text::Add(const PARSE_REC_INFO2_ITEM& item)
{
	map<DWORD, PARSE_REC_INFO2_ITEM>::const_iterator itr =
		this->itemMap.insert(pair<DWORD, PARSE_REC_INFO2_ITEM>(this->itemMap.empty() ? 1 : this->itemMap.rbegin()->first + 1, item)).first;
	return itr->first;
}

bool CParseRecInfo2Text::ParseLine(LPCWSTR parseLine, pair<DWORD, PARSE_REC_INFO2_ITEM>& item)
{
	if( wcschr(parseLine, L'\t') == NULL || parseLine[0] == L';' ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	item.second.originalNetworkID = (WORD)_wtoi(NextToken(token));
	item.second.transportStreamID = (WORD)_wtoi(NextToken(token));
	item.second.serviceID = (WORD)_wtoi(NextToken(token));

	FILETIME ft;
	item.second.startTime.wMilliseconds = 0;
	if( swscanf_s(NextToken(token), L"%hu/%hu/%hu", &item.second.startTime.wYear, &item.second.startTime.wMonth, &item.second.startTime.wDay) != 3 ||
	    swscanf_s(NextToken(token), L"%hu:%hu:%hu", &item.second.startTime.wHour, &item.second.startTime.wMinute, &item.second.startTime.wSecond) != 3 ||
	    SystemTimeToFileTime(&item.second.startTime, &ft) == FALSE ||
	    FileTimeToSystemTime(&ft, &item.second.startTime) == FALSE ){
		return false;
	}
	NextToken(token);
	item.second.eventName.assign(token[0], token[1]);
	item.first = this->itemMap.empty() ? 1 : this->itemMap.rbegin()->first + 1;
	return true;
}

bool CParseRecInfo2Text::SaveLine(const pair<DWORD, PARSE_REC_INFO2_ITEM>& item, wstring& saveLine) const
{
	Format(saveLine, L"%d\n%d\n%d\n%04d/%02d/%02d\n%02d:%02d:%02d\n%s",
		item.second.originalNetworkID,
		item.second.transportStreamID,
		item.second.serviceID,
		item.second.startTime.wYear, item.second.startTime.wMonth, item.second.startTime.wDay,
		item.second.startTime.wHour, item.second.startTime.wMinute, item.second.startTime.wSecond,
		item.second.eventName.c_str()
		);
	return FinalizeField(saveLine) == 5;
}

bool CParseRecInfo2Text::SelectIDToSave(vector<DWORD>& sortList) const
{
	map<DWORD, PARSE_REC_INFO2_ITEM>::const_iterator itr = this->itemMap.begin();
	if( this->itemMap.size() > this->keepCount ){
		advance(itr, this->itemMap.size() - this->keepCount);
	}
	for( ; itr != this->itemMap.end(); itr++ ){
		sortList.push_back(itr->first);
	}
	return true;
}

DWORD CParseReserveText::AddReserve(const RESERVE_DATA& item)
{
	map<DWORD, RESERVE_DATA>::iterator itr = this->itemMap.insert(pair<DWORD, RESERVE_DATA>(this->nextID, item)).first;
	this->nextID = this->nextID % 100000000 + 1;
	this->sortByEventCache.clear();
	return itr->second.reserveID = itr->first;
}

bool CParseReserveText::ChgReserve(const RESERVE_DATA& item)
{
	map<DWORD, RESERVE_DATA>::iterator itr = this->itemMap.find(item.reserveID);
	if( itr != this->itemMap.end() ){
		itr->second = item;
		this->sortByEventCache.clear();
		return true;
	}
	return false;
}

bool CParseReserveText::SetPresentFlag(DWORD id, BYTE presentFlag)
{
	map<DWORD, RESERVE_DATA>::iterator itr = this->itemMap.find(id);
	if( itr != this->itemMap.end() ){
		itr->second.presentFlag = presentFlag;
		return true;
	}
	return false;
}

bool CParseReserveText::SetOverlapMode(DWORD id, BYTE overlapMode)
{
	map<DWORD, RESERVE_DATA>::iterator itr = this->itemMap.find(id);
	if( itr != this->itemMap.end() ){
		itr->second.overlapMode = overlapMode;
		return true;
	}
	return false;
}

bool CParseReserveText::DelReserve(DWORD id)
{
	if( this->itemMap.erase(id) != 0 ){
		this->sortByEventCache.clear();
		return true;
	}
	return false;
}

bool CParseReserveText::ParseLine(LPCWSTR parseLine, pair<DWORD, RESERVE_DATA>& item)
{
	if( this->saveNextID == 1 ){
		this->saveNextID = 0;
	}
	if( parseLine[0] == L';' ){
		if( wcsncmp(parseLine, L";;NextID=", 9) == 0 ){
			DWORD nextID_ = (DWORD)_wtoi(&parseLine[9]);
			if( nextID_ != 0 && nextID_ <= 100000000 ){
				this->nextID = this->nextID > nextID_ + 50000000 ? nextID_ : max(nextID_, this->nextID);
			}
			this->saveNextID = 2;
		}
		return false;
	}else if( wcschr(parseLine, L'\t') == NULL ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	FILETIME ft;
	item.second.startTime.wMilliseconds = 0;
	if( swscanf_s(NextToken(token), L"%hu/%hu/%hu", &item.second.startTime.wYear, &item.second.startTime.wMonth, &item.second.startTime.wDay) != 3 ||
	    swscanf_s(NextToken(token), L"%hu:%hu:%hu", &item.second.startTime.wHour, &item.second.startTime.wMinute, &item.second.startTime.wSecond) != 3 ||
	    SystemTimeToFileTime(&item.second.startTime, &ft) == FALSE ||
	    FileTimeToSystemTime(&ft, &item.second.startTime) == FALSE ){
		return false;
	}
	WORD wDuration[3];
	if( swscanf_s(NextToken(token), L"%hu:%hu:%hu", &wDuration[0], &wDuration[1], &wDuration[2]) != 3 ){
		return false;
	}
	item.second.durationSecond = (wDuration[0] * 60 + wDuration[1]) * 60 + wDuration[2];

	NextToken(token);
	item.second.title.assign(token[0], token[1]);
	NextToken(token);
	item.second.stationName.assign(token[0], token[1]);
	item.second.originalNetworkID = (WORD)_wtoi(NextToken(token));
	item.second.transportStreamID = (WORD)_wtoi(NextToken(token));
	item.second.serviceID = (WORD)_wtoi(NextToken(token));
	item.second.eventID = (WORD)_wtoi(NextToken(token));
	item.second.recSetting.priority = (BYTE)_wtoi(NextToken(token));
	item.second.recSetting.tuijyuuFlag = _wtoi(NextToken(token)) != 0 && item.second.eventID != 0xFFFF;
	item.second.reserveID = item.first = _wtoi(NextToken(token));
	if( item.first == 0 || item.first > 100000000 ){
		return false;
	}
	item.second.recSetting.recMode = (BYTE)_wtoi(NextToken(token));
	item.second.recSetting.pittariFlag = _wtoi(NextToken(token)) != 0 && item.second.eventID != 0xFFFF;
	NextToken(token);
	if( item.second.recSetting.batFilePath.assign(token[0], token[1]) == L"0" ){
		item.second.recSetting.batFilePath.clear();
	}
	//�����p
	NextToken(token);
	NextToken(token);
	item.second.comment.assign(token[0], token[1]);
	NextToken(token);
	vector<wstring> strRecFolderList(1, wstring(token[0], token[1]));
	item.second.recSetting.suspendMode = (BYTE)_wtoi(NextToken(token));
	if( token[0] == token[1] ){
		item.second.recSetting.suspendMode = 4;
	}
	item.second.recSetting.rebootFlag = _wtoi(NextToken(token)) != 0;
	//�p�~(��recFilePath)
	NextToken(token);
	item.second.recSetting.useMargineFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.startMargine = _wtoi(NextToken(token));
	item.second.recSetting.endMargine = _wtoi(NextToken(token));
	item.second.recSetting.serviceMode = _wtoi(NextToken(token));

	item.second.startTimeEpg.wMilliseconds = 0;
	if( swscanf_s(NextToken(token), L"%hu/%hu/%hu", &item.second.startTimeEpg.wYear, &item.second.startTimeEpg.wMonth, &item.second.startTimeEpg.wDay) != 3 ||
	    swscanf_s(NextToken(token), L"%hu:%hu:%hu", &item.second.startTimeEpg.wHour, &item.second.startTimeEpg.wMinute, &item.second.startTimeEpg.wSecond) != 3 ||
	    SystemTimeToFileTime(&item.second.startTimeEpg, &ft) == FALSE ||
	    FileTimeToSystemTime(&ft, &item.second.startTimeEpg) == FALSE ){
		return false;
	}
	for( DWORD recFolderNum = _wtoi(NextToken(token)); recFolderNum != 0; recFolderNum-- ){
		NextToken(token);
		strRecFolderList.push_back(wstring(token[0], token[1]));
	}
	for( size_t i = 0; i < strRecFolderList.size(); i++ ){
		REC_FILE_SET_INFO folderItem;
		if( strRecFolderList[i].empty() == false ){
			Separate(strRecFolderList[i], L"*", folderItem.recFolder, folderItem.writePlugIn);
			Separate(folderItem.writePlugIn, L"*", folderItem.writePlugIn, folderItem.recNamePlugIn);
			if( folderItem.writePlugIn.empty() ){
				folderItem.writePlugIn = L"Write_Default.dll";
			}
			item.second.recSetting.recFolderList.push_back(folderItem);
		}
	}
	item.second.recSetting.continueRecFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.partialRecFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.tunerID = _wtoi(NextToken(token));
	item.second.reserveStatus = _wtoi(NextToken(token));

	for( DWORD recFolderNum = _wtoi(NextToken(token)); recFolderNum != 0; recFolderNum-- ){
		REC_FILE_SET_INFO folderItem;
		NextToken(token);
		if( folderItem.recFolder.assign(token[0], token[1]).empty() == false ){
			Separate(folderItem.recFolder, L"*", folderItem.recFolder, folderItem.writePlugIn);
			Separate(folderItem.writePlugIn, L"*", folderItem.writePlugIn, folderItem.recNamePlugIn);
			if( folderItem.writePlugIn.empty() ){
				folderItem.writePlugIn = L"Write_Default.dll";
			}
			item.second.recSetting.partialRecFolder.push_back(folderItem);
		}
	}
	item.second.presentFlag = 0;
	item.second.overlapMode = 0;
	this->nextID = this->nextID > item.first + 50000000 ? item.first + 1 : (max(item.first + 1, this->nextID) - 1) % 100000000 + 1;
	this->sortByEventCache.clear();
	return true;
}

bool CParseReserveText::SaveLine(const pair<DWORD, RESERVE_DATA>& item, wstring& saveLine) const
{
	wstring strRecFolder;
	for( size_t i = 1; i < item.second.recSetting.recFolderList.size(); i++ ){
		strRecFolder +=
			item.second.recSetting.recFolderList[i].recFolder + L"*" +
			item.second.recSetting.recFolderList[i].writePlugIn + L"*" +
			item.second.recSetting.recFolderList[i].recNamePlugIn + L"\n";
	}
	wstring strPartialRecFolder;
	for( size_t i = 0; i < item.second.recSetting.partialRecFolder.size(); i++ ){
		strPartialRecFolder +=
			item.second.recSetting.partialRecFolder[i].recFolder + L"*" +
			item.second.recSetting.partialRecFolder[i].writePlugIn + L"*" +
			item.second.recSetting.partialRecFolder[i].recNamePlugIn + L"\n";
	}
	Format(saveLine, L"%04d/%02d/%02d\n%02d:%02d:%02d\n%02d:%02d:%02d\n%s\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%s\n%s\n%s\n%s\n%d\n%d\n%s\n%d\n%d\n%d\n%d\n%04d/%02d/%02d\n%02d:%02d:%02d\n%d\n%s%d\n%d\n%d\n%d\n%d\n%s",
		item.second.startTime.wYear, item.second.startTime.wMonth, item.second.startTime.wDay,
		item.second.startTime.wHour, item.second.startTime.wMinute, item.second.startTime.wSecond,
		item.second.durationSecond / 60 / 60, item.second.durationSecond / 60 % 60, item.second.durationSecond % 60,
		item.second.title.c_str(),
		item.second.stationName.c_str(),
		item.second.originalNetworkID,
		item.second.transportStreamID,
		item.second.serviceID,
		item.second.eventID,
		item.second.recSetting.priority,
		item.second.recSetting.tuijyuuFlag,
		item.second.reserveID,
		item.second.recSetting.recMode,
		item.second.recSetting.pittariFlag,
		item.second.recSetting.batFilePath.empty() ? L"0" : item.second.recSetting.batFilePath.c_str(),
		L"0",
		item.second.comment.c_str(),
		item.second.recSetting.recFolderList.empty() ? L"" : (
			item.second.recSetting.recFolderList[0].recFolder + L"*" +
			item.second.recSetting.recFolderList[0].writePlugIn + L"*" +
			item.second.recSetting.recFolderList[0].recNamePlugIn).c_str(),
		item.second.recSetting.suspendMode,
		item.second.recSetting.rebootFlag,
		L"",
		item.second.recSetting.useMargineFlag,
		item.second.recSetting.startMargine,
		item.second.recSetting.endMargine,
		item.second.recSetting.serviceMode,
		item.second.startTimeEpg.wYear, item.second.startTimeEpg.wMonth, item.second.startTimeEpg.wDay,
		item.second.startTimeEpg.wHour, item.second.startTimeEpg.wMinute, item.second.startTimeEpg.wSecond,
		(int)(item.second.recSetting.recFolderList.empty() ? 0 : item.second.recSetting.recFolderList.size() - 1),
		strRecFolder.c_str(),
		item.second.recSetting.continueRecFlag,
		item.second.recSetting.partialRecFlag,
		item.second.recSetting.tunerID,
		item.second.reserveStatus,
		(int)item.second.recSetting.partialRecFolder.size(),
		strPartialRecFolder.c_str()
		);
	return FinalizeField(saveLine) == 33 + max((int)item.second.recSetting.recFolderList.size() - 1, 0) + item.second.recSetting.partialRecFolder.size();
}

bool CParseReserveText::SaveFooterLine(wstring& saveLine) const
{
	//���̓ǂݍ��ݎ���nextID�𕜌����邽�߂̃t�b�^�R�����g
	//���̃R�����g�͂����폜����Ă��傫�Ȗ��͂Ȃ�
	Format(saveLine, L";;NextID=%d", this->nextID);
	//�ǂݍ��ݎ��ɂ��̃R�����g�����������Ƃ��͕ۑ����Ȃ�
	return this->saveNextID != 0;
}

bool CParseReserveText::SelectIDToSave(vector<DWORD>& sortList) const
{
	if( this->saveNextID == 0 ){
		//NextID�R�����g�����������Ƃ��͏]���ǂ���\��������ŕۑ�����
		vector<pair<LONGLONG, const RESERVE_DATA*>> sortItemList = GetReserveList();
		vector<pair<LONGLONG, const RESERVE_DATA*>>::const_iterator itr;
		for( itr = sortItemList.begin(); itr != sortItemList.end(); itr++ ){
			sortList.push_back(itr->second->reserveID);
		}
		return true;
	}
	if( this->itemMap.empty() == false && this->itemMap.rbegin()->first >= this->itemMap.begin()->first + 50000000 ){
		//ID����
		map<DWORD, RESERVE_DATA>::const_iterator itr;
		for( itr = this->itemMap.upper_bound(50000000); itr != this->itemMap.end(); itr++ ){
			sortList.push_back(itr->first);
		}
		for( itr = this->itemMap.begin(); itr->first <= 50000000; itr++ ){
			sortList.push_back(itr->first);
		}
		return true;
	}
	return false;
}

vector<pair<LONGLONG, const RESERVE_DATA*>> CParseReserveText::GetReserveList(BOOL calcMargin, int defStartMargin) const
{
	vector<pair<LONGLONG, const RESERVE_DATA*>> retList;
	retList.reserve(this->itemMap.size());

	//�������Ƀ\�[�g
	map<DWORD, RESERVE_DATA>::const_iterator itr;
	for( itr = this->itemMap.begin(); itr != this->itemMap.end(); itr++ ){
		LONGLONG startTime = ConvertI64Time(itr->second.startTime);
		if( calcMargin != FALSE ){
			LONGLONG endTime = startTime + itr->second.durationSecond * I64_1SEC;
			LONGLONG startMargin = defStartMargin * I64_1SEC;
			if( itr->second.recSetting.useMargineFlag == TRUE ){
				startMargin = itr->second.recSetting.startMargine * I64_1SEC;
			}
			//�J�n�}�[�W���͌��̗\��I�������𒴂��ĕ��ł����Ă͂Ȃ�Ȃ�
			startTime -= max(startMargin, startTime - endTime);
		}
		retList.push_back( pair<LONGLONG, const RESERVE_DATA*>((startTime / I64_1SEC) << 16 | itr->second.transportStreamID, &itr->second) );
	}
	std::sort(retList.begin(), retList.end());
	return retList;
}

const vector<pair<ULONGLONG, DWORD>>& CParseReserveText::GetSortByEventList() const
{
	if( this->sortByEventCache.empty() || this->itemMap.empty() ){
		this->sortByEventCache.clear();
		this->sortByEventCache.reserve(this->itemMap.size());
		for( map<DWORD, RESERVE_DATA>::const_iterator itr = this->itemMap.begin(); itr != this->itemMap.end(); itr++ ){
			this->sortByEventCache.push_back(std::make_pair(
				(ULONGLONG)itr->second.originalNetworkID << 48 | (ULONGLONG)itr->second.transportStreamID << 32 |
				(DWORD)itr->second.serviceID << 16 | itr->second.eventID, itr->first));
		}
		std::sort(this->sortByEventCache.begin(), this->sortByEventCache.end());
	}
	return this->sortByEventCache;
}

DWORD CParseEpgAutoAddText::AddData(const EPG_AUTO_ADD_DATA& item)
{
	map<DWORD, EPG_AUTO_ADD_DATA>::iterator itr = this->itemMap.insert(pair<DWORD, EPG_AUTO_ADD_DATA>(this->nextID, item)).first;
	this->nextID = this->nextID % 100000000 + 1;
	return itr->second.dataID = itr->first;
}

bool CParseEpgAutoAddText::ChgData(const EPG_AUTO_ADD_DATA& item)
{
	map<DWORD, EPG_AUTO_ADD_DATA>::iterator itr = this->itemMap.find(item.dataID);
	if( itr != this->itemMap.end() ){
		itr->second = item;
		return true;
	}
	return false;
}

bool CParseEpgAutoAddText::SetAddCount(DWORD id, DWORD addCount)
{
	map<DWORD, EPG_AUTO_ADD_DATA>::iterator itr = this->itemMap.find(id);
	if( itr != this->itemMap.end() ){
		itr->second.addCount = addCount;
		return true;
	}
	return false;
}

bool CParseEpgAutoAddText::DelData(DWORD id)
{
	return this->itemMap.erase(id) != 0;
}

bool CParseEpgAutoAddText::ParseLine(LPCWSTR parseLine, pair<DWORD, EPG_AUTO_ADD_DATA>& item)
{
	if( this->saveNextID == 1 ){
		this->saveNextID = 0;
	}
	if( parseLine[0] == L';' ){
		if( wcsncmp(parseLine, L";;NextID=", 9) == 0 ){
			DWORD nextID_ = (DWORD)_wtoi(&parseLine[9]);
			if( nextID_ != 0 && nextID_ <= 100000000 ){
				this->nextID = this->nextID > nextID_ + 50000000 ? nextID_ : max(nextID_, this->nextID);
			}
			this->saveNextID = 2;
		}
		return false;
	}else if( wcschr(parseLine, L'\t') == NULL ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	item.second.dataID = item.first = _wtoi(NextToken(token));
	if( item.first == 0 || item.first > 100000000 ){
		return false;
	}
	NextToken(token);
	item.second.searchInfo.andKey.assign(token[0], token[1]);
	NextToken(token);
	item.second.searchInfo.notKey.assign(token[0], token[1]);
	item.second.searchInfo.regExpFlag = _wtoi(NextToken(token)) != 0;
	item.second.searchInfo.titleOnlyFlag = _wtoi(NextToken(token)) != 0;
	NextToken(token);
	wstring strContent(token[0], token[1]);
	for( size_t i = 0; i != wstring::npos; i = strContent.find(L',', i + 1) ){
		int flag = 0;
		//����: �݊��̂���"%d"
		if( swscanf_s(&strContent.c_str()[i == 0 ? 0 : i + 1], L"%d", &flag) == 1 ){
			EPGDB_CONTENT_DATA addItem;
			addItem.content_nibble_level_1 = (BYTE)((DWORD)flag >> 24);
			addItem.content_nibble_level_2 = (BYTE)((DWORD)flag >> 16);
			addItem.user_nibble_1 = (BYTE)((DWORD)flag >> 8);
			addItem.user_nibble_2 = (BYTE)((DWORD)flag);
			item.second.searchInfo.contentList.push_back(addItem);
		}
	}
	NextToken(token);
	wstring strDate(token[0], token[1]);
	for( size_t i = 0; i != wstring::npos; i = strDate.find(L',', i + 1) ){
		DWORD dwTime[4];
		if( swscanf_s(&strDate.c_str()[i == 0 ? 0 : i + 1], L"%u-%u-%u-%u", &dwTime[0], &dwTime[1], &dwTime[2], &dwTime[3]) == 4 ){
			EPGDB_SEARCH_DATE_INFO addItem;
			addItem.startDayOfWeek = dwTime[0] % 7;
			addItem.startHour = (dwTime[1] >> 16) % 24;
			addItem.startMin = (dwTime[1] & 0xFFFF) % 60;
			addItem.endDayOfWeek = dwTime[2] % 7;
			addItem.endHour = (dwTime[3] >> 16) % 24;
			addItem.endMin = (dwTime[3] & 0xFFFF) % 60;
			item.second.searchInfo.dateList.push_back(addItem);
		}
	}
	NextToken(token);
	wstring strService(token[0], token[1]);
	for( size_t i = 0; i != wstring::npos; i = strService.find(L',', i + 1) ){
		__int64 i64Ch = 0;
		if( swscanf_s(&strService.c_str()[i == 0 ? 0 : i + 1], L"%I64X", &i64Ch) == 1 ){
			item.second.searchInfo.serviceList.push_back(i64Ch & 0xFFFFFFFFFFFFLL);
		}
	}
	item.second.recSetting.recMode = (BYTE)_wtoi(NextToken(token));
	item.second.recSetting.priority = (BYTE)_wtoi(NextToken(token));
	item.second.recSetting.tuijyuuFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.serviceMode = _wtoi(NextToken(token));
	item.second.recSetting.pittariFlag = _wtoi(NextToken(token)) != 0;
	NextToken(token);
	item.second.recSetting.batFilePath.assign(token[0], token[1]);
	item.second.recSetting.suspendMode = (BYTE)_wtoi(NextToken(token));
	if( token[0] == token[1] ){
		item.second.recSetting.suspendMode = 4;
	}
	item.second.recSetting.rebootFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.useMargineFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.startMargine = _wtoi(NextToken(token));
	item.second.recSetting.endMargine = _wtoi(NextToken(token));

	for( DWORD recFolderNum = _wtoi(NextToken(token)); recFolderNum != 0; recFolderNum-- ){
		REC_FILE_SET_INFO folderItem;
		NextToken(token);
		if( folderItem.recFolder.assign(token[0], token[1]).empty() == false ){
			Separate(folderItem.recFolder, L"*", folderItem.recFolder, folderItem.writePlugIn);
			Separate(folderItem.writePlugIn, L"*", folderItem.writePlugIn, folderItem.recNamePlugIn);
			if( folderItem.writePlugIn.empty() ){
				folderItem.writePlugIn = L"Write_Default.dll";
			}
			item.second.recSetting.recFolderList.push_back(folderItem);
		}
	}
	item.second.recSetting.continueRecFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.partialRecFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.tunerID = _wtoi(NextToken(token));
	item.second.searchInfo.aimaiFlag = _wtoi(NextToken(token)) != 0;
	item.second.searchInfo.notContetFlag = _wtoi(NextToken(token)) != 0;
	item.second.searchInfo.notDateFlag = _wtoi(NextToken(token)) != 0;
	item.second.searchInfo.freeCAFlag = (BYTE)_wtoi(NextToken(token));

	for( DWORD recFolderNum = _wtoi(NextToken(token)); recFolderNum != 0; recFolderNum-- ){
		REC_FILE_SET_INFO folderItem;
		NextToken(token);
		if( folderItem.recFolder.assign(token[0], token[1]).empty() == false ){
			Separate(folderItem.recFolder, L"*", folderItem.recFolder, folderItem.writePlugIn);
			Separate(folderItem.writePlugIn, L"*", folderItem.writePlugIn, folderItem.recNamePlugIn);
			if( folderItem.writePlugIn.empty() ){
				folderItem.writePlugIn = L"Write_Default.dll";
			}
			item.second.recSetting.partialRecFolder.push_back(folderItem);
		}
	}
	item.second.searchInfo.chkRecEnd = _wtoi(NextToken(token)) != 0;
	item.second.searchInfo.chkRecDay = (WORD)_wtoi(NextToken(token));
	item.second.addCount = 0;
	this->nextID = this->nextID > item.first + 50000000 ? item.first + 1 : (max(item.first + 1, this->nextID) - 1) % 100000000 + 1;
	return true;
}

bool CParseEpgAutoAddText::SaveLine(const pair<DWORD, EPG_AUTO_ADD_DATA>& item, wstring& saveLine) const
{
	wstring strContent;
	for( size_t i = 0; i < item.second.searchInfo.contentList.size(); i++ ){
		WCHAR s[64];
		swprintf_s(s, L"%d",
			(DWORD)item.second.searchInfo.contentList[i].content_nibble_level_1 << 24 |
			item.second.searchInfo.contentList[i].content_nibble_level_2 << 16 |
			item.second.searchInfo.contentList[i].user_nibble_1 << 8 |
			item.second.searchInfo.contentList[i].user_nibble_2);
		if( i != 0 ){
			strContent += L',';
		}
		strContent += s;
	}
	wstring strDate;
	for( size_t i = 0; i < item.second.searchInfo.dateList.size(); i++ ){
		WCHAR s[64];
		swprintf_s(s, L"%d-%u-%d-%u",
			item.second.searchInfo.dateList[i].startDayOfWeek,
			(DWORD)item.second.searchInfo.dateList[i].startHour << 16 | item.second.searchInfo.dateList[i].startMin,
			item.second.searchInfo.dateList[i].endDayOfWeek,
			(DWORD)item.second.searchInfo.dateList[i].endHour << 16 | item.second.searchInfo.dateList[i].endMin);
		if( i != 0 ){
			strDate += L',';
		}
		strDate += s;
	}
	wstring strService;
	for( size_t i = 0; i < item.second.searchInfo.serviceList.size(); i++ ){
		WCHAR s[64];
		swprintf_s(s, L"%012I64X", item.second.searchInfo.serviceList[i]);
		if( i != 0 ){
			strService += L',';
		}
		strService += s;
	}
	wstring strRecFolder;
	for( size_t i = 0; i < item.second.recSetting.recFolderList.size(); i++ ){
		strRecFolder +=
			item.second.recSetting.recFolderList[i].recFolder + L"*" +
			item.second.recSetting.recFolderList[i].writePlugIn + L"*" +
			item.second.recSetting.recFolderList[i].recNamePlugIn + L"\n";
	}
	wstring strPartialRecFolder;
	for( size_t i = 0; i < item.second.recSetting.partialRecFolder.size(); i++ ){
		strPartialRecFolder +=
			item.second.recSetting.partialRecFolder[i].recFolder + L"*" +
			item.second.recSetting.partialRecFolder[i].writePlugIn + L"*" +
			item.second.recSetting.partialRecFolder[i].recNamePlugIn + L"\n";
	}
	Format(saveLine, L"%d\n%s\n%s\n%d\n%d\n%s\n%s\n%s\n%d\n%d\n%d\n%d\n%d\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%s%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%s%d\n%d\n",
		item.second.dataID,
		item.second.searchInfo.andKey.c_str(),
		item.second.searchInfo.notKey.c_str(),
		item.second.searchInfo.regExpFlag,
		item.second.searchInfo.titleOnlyFlag,
		strContent.c_str(),
		strDate.c_str(),
		strService.c_str(),
		item.second.recSetting.recMode,
		item.second.recSetting.priority,
		item.second.recSetting.tuijyuuFlag,
		item.second.recSetting.serviceMode,
		item.second.recSetting.pittariFlag,
		item.second.recSetting.batFilePath.c_str(),
		item.second.recSetting.suspendMode,
		item.second.recSetting.rebootFlag,
		item.second.recSetting.useMargineFlag,
		item.second.recSetting.startMargine,
		item.second.recSetting.endMargine,
		(int)item.second.recSetting.recFolderList.size(),
		strRecFolder.c_str(),
		item.second.recSetting.continueRecFlag,
		item.second.recSetting.partialRecFlag,
		item.second.recSetting.tunerID,
		item.second.searchInfo.aimaiFlag,
		item.second.searchInfo.notContetFlag,
		item.second.searchInfo.notDateFlag,
		item.second.searchInfo.freeCAFlag,
		(int)item.second.recSetting.partialRecFolder.size(),
		strPartialRecFolder.c_str(),
		item.second.searchInfo.chkRecEnd,
		item.second.searchInfo.chkRecDay
		);
	return FinalizeField(saveLine) == 30 + item.second.recSetting.recFolderList.size() + item.second.recSetting.partialRecFolder.size();
}

bool CParseEpgAutoAddText::SaveFooterLine(wstring& saveLine) const
{
	Format(saveLine, L";;NextID=%d", this->nextID);
	return this->saveNextID != 0;
}

bool CParseEpgAutoAddText::SelectIDToSave(vector<DWORD>& sortList) const
{
	if( this->itemMap.empty() == false && this->itemMap.rbegin()->first >= this->itemMap.begin()->first + 50000000 ){
		map<DWORD, EPG_AUTO_ADD_DATA>::const_iterator itr;
		for( itr = this->itemMap.upper_bound(50000000); itr != this->itemMap.end(); itr++ ){
			sortList.push_back(itr->first);
		}
		for( itr = this->itemMap.begin(); itr->first <= 50000000; itr++ ){
			sortList.push_back(itr->first);
		}
		return true;
	}
	return false;
}

DWORD CParseManualAutoAddText::AddData(const MANUAL_AUTO_ADD_DATA& item)
{
	map<DWORD, MANUAL_AUTO_ADD_DATA>::iterator itr = this->itemMap.insert(pair<DWORD, MANUAL_AUTO_ADD_DATA>(this->nextID, item)).first;
	this->nextID = this->nextID % 100000000 + 1;
	itr->second.recSetting.pittariFlag = 0;
	itr->second.recSetting.tuijyuuFlag = 0;
	return itr->second.dataID = itr->first;
}

bool CParseManualAutoAddText::ChgData(const MANUAL_AUTO_ADD_DATA& item)
{
	map<DWORD, MANUAL_AUTO_ADD_DATA>::iterator itr = this->itemMap.find(item.dataID);
	if( itr != this->itemMap.end() ){
		itr->second = item;
		itr->second.recSetting.pittariFlag = 0;
		itr->second.recSetting.tuijyuuFlag = 0;
		return true;
	}
	return false;
}

bool CParseManualAutoAddText::DelData(DWORD id)
{
	return this->itemMap.erase(id) != 0;
}

bool CParseManualAutoAddText::ParseLine(LPCWSTR parseLine, pair<DWORD, MANUAL_AUTO_ADD_DATA>& item)
{
	if( this->saveNextID == 1 ){
		this->saveNextID = 0;
	}
	if( parseLine[0] == L';' ){
		if( wcsncmp(parseLine, L";;NextID=", 9) == 0 ){
			DWORD nextID_ = (DWORD)_wtoi(&parseLine[9]);
			if( nextID_ != 0 && nextID_ <= 100000000 ){
				this->nextID = this->nextID > nextID_ + 50000000 ? nextID_ : max(nextID_, this->nextID);
			}
			this->saveNextID = 2;
		}
		return false;
	}else if( wcschr(parseLine, L'\t') == NULL ){
		return false;
	}
	LPCWSTR token[3] = {NULL, NULL, parseLine};

	item.second.dataID = item.first = _wtoi(NextToken(token));
	if( item.first == 0 || item.first > 100000000 ){
		return false;
	}
	item.second.dayOfWeekFlag = (BYTE)_wtoi(NextToken(token));
	item.second.startTime = _wtoi(NextToken(token));
	item.second.durationSecond = _wtoi(NextToken(token));
	NextToken(token);
	item.second.title.assign(token[0], token[1]);
	NextToken(token);
	item.second.stationName.assign(token[0], token[1]);
	item.second.originalNetworkID = (WORD)_wtoi(NextToken(token));
	item.second.transportStreamID = (WORD)_wtoi(NextToken(token));
	item.second.serviceID = (WORD)_wtoi(NextToken(token));
	item.second.recSetting.recMode = (BYTE)_wtoi(NextToken(token));
	item.second.recSetting.priority = (BYTE)_wtoi(NextToken(token));
	item.second.recSetting.tuijyuuFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.serviceMode = _wtoi(NextToken(token));
	item.second.recSetting.pittariFlag = _wtoi(NextToken(token)) != 0;
	NextToken(token);
	item.second.recSetting.batFilePath.assign(token[0], token[1]);
	item.second.recSetting.suspendMode = (BYTE)_wtoi(NextToken(token));
	if( token[0] == token[1] ){
		item.second.recSetting.suspendMode = 4;
	}
	item.second.recSetting.rebootFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.useMargineFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.startMargine = _wtoi(NextToken(token));
	item.second.recSetting.endMargine = _wtoi(NextToken(token));

	for( DWORD recFolderNum = _wtoi(NextToken(token)); recFolderNum != 0; recFolderNum-- ){
		REC_FILE_SET_INFO folderItem;
		NextToken(token);
		if( folderItem.recFolder.assign(token[0], token[1]).empty() == false ){
			Separate(folderItem.recFolder, L"*", folderItem.recFolder, folderItem.writePlugIn);
			Separate(folderItem.writePlugIn, L"*", folderItem.writePlugIn, folderItem.recNamePlugIn);
			if( folderItem.writePlugIn.empty() ){
				folderItem.writePlugIn = L"Write_Default.dll";
			}
			item.second.recSetting.recFolderList.push_back(folderItem);
		}
	}
	item.second.recSetting.continueRecFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.partialRecFlag = _wtoi(NextToken(token)) != 0;
	item.second.recSetting.tunerID = _wtoi(NextToken(token));

	for( DWORD recFolderNum = _wtoi(NextToken(token)); recFolderNum != 0; recFolderNum-- ){
		REC_FILE_SET_INFO folderItem;
		NextToken(token);
		if( folderItem.recFolder.assign(token[0], token[1]).empty() == false ){
			Separate(folderItem.recFolder, L"*", folderItem.recFolder, folderItem.writePlugIn);
			Separate(folderItem.writePlugIn, L"*", folderItem.writePlugIn, folderItem.recNamePlugIn);
			if( folderItem.writePlugIn.empty() ){
				folderItem.writePlugIn = L"Write_Default.dll";
			}
			item.second.recSetting.partialRecFolder.push_back(folderItem);
		}
	}
	this->nextID = this->nextID > item.first + 50000000 ? item.first + 1 : (max(item.first + 1, this->nextID) - 1) % 100000000 + 1;
	return true;
}

bool CParseManualAutoAddText::SaveLine(const pair<DWORD, MANUAL_AUTO_ADD_DATA>& item, wstring& saveLine) const
{
	wstring strRecFolder;
	for( size_t i = 0; i < item.second.recSetting.recFolderList.size(); i++ ){
		strRecFolder +=
			item.second.recSetting.recFolderList[i].recFolder + L"*" +
			item.second.recSetting.recFolderList[i].writePlugIn + L"*" +
			item.second.recSetting.recFolderList[i].recNamePlugIn + L"\n";
	}
	wstring strPartialRecFolder;
	for( size_t i = 0; i < item.second.recSetting.partialRecFolder.size(); i++ ){
		strPartialRecFolder +=
			item.second.recSetting.partialRecFolder[i].recFolder + L"*" +
			item.second.recSetting.partialRecFolder[i].writePlugIn + L"*" +
			item.second.recSetting.partialRecFolder[i].recNamePlugIn + L"\n";
	}
	Format(saveLine, L"%d\n%d\n%d\n%d\n%s\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%s%d\n%d\n%d\n%d\n%s",
		item.second.dataID,
		item.second.dayOfWeekFlag,
		item.second.startTime,
		item.second.durationSecond,
		item.second.title.c_str(),
		item.second.stationName.c_str(),
		item.second.originalNetworkID,
		item.second.transportStreamID,
		item.second.serviceID,
		item.second.recSetting.recMode,
		item.second.recSetting.priority,
		item.second.recSetting.tuijyuuFlag,
		item.second.recSetting.serviceMode,
		item.second.recSetting.pittariFlag,
		item.second.recSetting.batFilePath.c_str(),
		item.second.recSetting.suspendMode,
		item.second.recSetting.rebootFlag,
		item.second.recSetting.useMargineFlag,
		item.second.recSetting.startMargine,
		item.second.recSetting.endMargine,
		(int)item.second.recSetting.recFolderList.size(),
		strRecFolder.c_str(),
		item.second.recSetting.continueRecFlag,
		item.second.recSetting.partialRecFlag,
		item.second.recSetting.tunerID,
		(int)item.second.recSetting.partialRecFolder.size(),
		strPartialRecFolder.c_str()
		);
	return FinalizeField(saveLine) == 25 + item.second.recSetting.recFolderList.size() + item.second.recSetting.partialRecFolder.size();
}

bool CParseManualAutoAddText::SaveFooterLine(wstring& saveLine) const
{
	Format(saveLine, L";;NextID=%d", this->nextID);
	return this->saveNextID != 0;
}

bool CParseManualAutoAddText::SelectIDToSave(vector<DWORD>& sortList) const
{
	if( this->itemMap.empty() == false && this->itemMap.rbegin()->first >= this->itemMap.begin()->first + 50000000 ){
		map<DWORD, MANUAL_AUTO_ADD_DATA>::const_iterator itr;
		for( itr = this->itemMap.upper_bound(50000000); itr != this->itemMap.end(); itr++ ){
			sortList.push_back(itr->first);
		}
		for( itr = this->itemMap.begin(); itr->first <= 50000000; itr++ ){
			sortList.push_back(itr->first);
		}
		return true;
	}
	return false;
}
