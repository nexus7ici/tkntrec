#pragma once

#include "PathUtil.h"
#include "StringUtil.h"
#include <stdio.h>

template <class K, class V>
class CParseText
{
public:
	CParseText() : isUtf8(false) {}
	virtual ~CParseText() {}
	bool ParseText(LPCWSTR path = NULL);
	const map<K, V>& GetMap() const { return this->itemMap; }
	const wstring& GetFilePath() const { return this->filePath; }
	void SetFilePath(LPCWSTR path) { this->filePath = path; this->isUtf8 = IsUtf8Default(); }
protected:
	bool SaveText() const;
	virtual bool ParseLine(LPCWSTR parseLine, pair<K, V>& item) = 0;
	virtual bool SaveLine(const pair<K, V>& item, wstring& saveLine) const { return false; }
	virtual bool SaveFooterLine(wstring& saveLine) const { return false; }
	virtual bool SelectIDToSave(vector<K>& sortList) const { return false; }
	virtual bool IsUtf8Default() const { return false; }
	map<K, V> itemMap;
	wstring filePath;
	bool isUtf8;
};

template <class K, class V>
bool CParseText<K, V>::ParseText(LPCWSTR path)
{
	this->itemMap.clear();
	this->isUtf8 = IsUtf8Default();
	if( path != NULL ){
		this->filePath = path;
	}
	if( this->filePath.empty() ){
		return false;
	}
	std::unique_ptr<FILE, decltype(&fclose)> fp(NULL, fclose);
	for( int retry = 0;; ){
		FILE* fp_;
		errno_t err = _wfopen_s(&fp_, this->filePath.c_str(), L"rbN");
		if( err == 0 ){
			fp.reset(fp_);
			break;
		}else if( err == ENOENT ){
			return true;
		}else if( ++retry > 5 ){
			//6��g���C���Ă���ł��_���Ȃ玸�s
			OutputDebugString(L"CParseText<>::ParseText(): Error: Cannot open file\r\n");
			return false;
		}
		Sleep(200 * retry);
	}

	this->isUtf8 = false;
	bool checkBom = false;
	vector<char> buf;
	vector<WCHAR> parseBuf;
	for(;;){
		//4KB�P�ʂœǂݍ���
		buf.resize(buf.size() + 4096);
		size_t n = fread(&buf.front() + buf.size() - 4096, 1, 4096, fp.get());
		if( n == 0 ){
			buf.resize(buf.size() - 4096);
			buf.push_back('\0');
		}else{
			buf.resize(buf.size() - 4096 + n);
		}
		if( checkBom == false ){
			checkBom = true;
			if( buf.size() >= 3 && buf[0] == '\xEF' && buf[1] == '\xBB' && buf[2] == '\xBF' ){
				this->isUtf8 = true;
				buf.erase(buf.begin(), buf.begin() + 3);
			}
		}
		//���S�ɓǂݍ��܂ꂽ�s���ł��邾�����
		size_t offset = 0;
		for( size_t i = 0; i < buf.size(); i++ ){
			bool eof = buf[i] == '\0';
			if( eof || buf[i] == '\r' && i + 1 < buf.size() && buf[i + 1] == '\n' ){
				buf[i] = '\0';
				if( this->isUtf8 ){
					UTF8toW(&buf[offset], i - offset, parseBuf);
				}else{
					AtoW(&buf[offset], i - offset, parseBuf);
				}
				pair<K, V> item;
				if( ParseLine(&parseBuf.front(), item) ){
					this->itemMap.insert(item);
				}
				if( eof ){
					offset = i;
					break;
				}
				offset = (++i) + 1;
			}
		}
		buf.erase(buf.begin(), buf.begin() + offset);
		if( buf.empty() == false && buf[0] == '\0' ){
			break;
		}
	}
	return true;
}

template <class K, class V>
bool CParseText<K, V>::SaveText() const
{
	if( this->filePath.empty() ){
		return false;
	}
	std::unique_ptr<FILE, decltype(&fclose)> fp(NULL, fclose);
	for( int retry = 0;; ){
		UtilCreateDirectories(fs_path(this->filePath).parent_path());
		FILE* fp_;
		if( _wfopen_s(&fp_, (this->filePath + L".tmp").c_str(), L"wbN") == 0 ){
			fp.reset(fp_);
			break;
		}else if( ++retry > 5 ){
			OutputDebugString(L"CParseText<>::SaveText(): Error: Cannot open file\r\n");
			return false;
		}
		Sleep(200 * retry);
	}

	bool ret = true;
	if( this->isUtf8 ){
		ret = ret && fputs("\xEF\xBB\xBF", fp.get()) >= 0;
	}
	wstring saveLine;
	vector<char> saveBuf;
	vector<K> idList;
	if( SelectIDToSave(idList) ){
		for( size_t i = 0; i < idList.size(); i++ ){
			auto itr = this->itemMap.find(idList[i]);
			saveLine.clear();
			if( itr != this->itemMap.end() && SaveLine(*itr, saveLine) ){
				saveLine += L"\r\n";
				size_t len;
				if( this->isUtf8 ){
					len = WtoUTF8(saveLine.c_str(), saveLine.size(), saveBuf);
				}else{
					len = WtoA(saveLine.c_str(), saveLine.size(), saveBuf);
				}
				ret = ret && fwrite(&saveBuf.front(), 1, len, fp.get()) == len;
			}
		}
	}else{
		for( auto itr = this->itemMap.cbegin(); itr != this->itemMap.end(); itr++ ){
			saveLine.clear();
			if( SaveLine(*itr, saveLine) ){
				saveLine += L"\r\n";
				size_t len;
				if( this->isUtf8 ){
					len = WtoUTF8(saveLine.c_str(), saveLine.size(), saveBuf);
				}else{
					len = WtoA(saveLine.c_str(), saveLine.size(), saveBuf);
				}
				ret = ret && fwrite(&saveBuf.front(), 1, len, fp.get()) == len;
			}
		}
	}
	saveLine.clear();
	if( SaveFooterLine(saveLine) ){
		saveLine += L"\r\n";
		size_t len;
		if( this->isUtf8 ){
			len = WtoUTF8(saveLine.c_str(), saveLine.size(), saveBuf);
		}else{
			len = WtoA(saveLine.c_str(), saveLine.size(), saveBuf);
		}
		ret = ret && fwrite(&saveBuf.front(), 1, len, fp.get()) == len;
	}
	fp.reset();

	if( ret ){
		for( int retry = 0;; ){
			if( MoveFileEx((this->filePath + L".tmp").c_str(), this->filePath.c_str(), MOVEFILE_REPLACE_EXISTING) ){
				return true;
			}else if( ++retry > 5 ){
				OutputDebugString(L"CParseText<>::SaveText(): Error: Cannot open file\r\n");
				break;
			}
			Sleep(200 * retry);
		}
	}
	return false;
}
