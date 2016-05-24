#include "cache.h"
#include "errorlogger.h"
#include <cryptopp/sha3.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <fstream>
using namespace std;
using namespace tinyxml2;
using namespace CryptoPP;

Cache::Cache()
{
}

Cache::Cache(const Cache & other)
{
	*this = other;
}

Cache & Cache::operator=(const Cache & other)
{
	Clear();
	if (!_cacheFile.empty())
	{
		Load(other._cacheFile);
	}
	return *this;
}

int Cache::Load(const string& file)
{
	XMLError er = XML_SUCCESS;

	if (!_cacheFile.empty())
	{
		return -1;
	}

	_cacheFile = file;
	er = _cache.LoadFile(file.c_str());

	return((er == XML_SUCCESS) || (er == XML_NO_ERROR)) ? 0 : (er | XML_SUCCESS);
}

int Cache::LoadFromCache(const char* filePath, const char* configuration, size_t* pCachedSize, std::string* pHash, const tinyxml2::XMLElement** ppElem) const
{
	const XMLElement *elem = NULL;
	const char* sizeStr = NULL;
	const char* hashStr = NULL;
	size_t tmpSize = 0;
	string path;
	(*pCachedSize) = 0;
	(*ppElem) = NULL;
	pHash->clear();

	path = Normalize(filePath);

	if ((Find(path.c_str(), configuration, &elem) != 0) || (elem == NULL))
	{
		return -1;
	}

	// Parse size
	if ((sizeStr = elem->Attribute("Size")) == NULL)
	{
		return -1;
	}

	if ((tmpSize = ::stoull(sizeStr, NULL)) == 0)
	{
		return -1;
	}

	// Hash
	if ((hashStr = elem->Attribute("Hash")) == NULL)
	{
		return -1;
	}

	(*pCachedSize) = tmpSize;
	(*pHash) = hashStr;
	(*ppElem) = elem;
	return 0;
}

int Cache::Find(const char* filePath, const char* configuration, XMLElement** ppElem)
{
	return Find(filePath, configuration, (const XMLElement**)ppElem);
}

int Cache::Find(const char* filePath, const char* configuration, const XMLElement** ppElem) const
{
	(*ppElem) = NULL;

	string path = Normalize(filePath);
	if (path.empty() || _cacheFile.empty())
	{
		return -1;
	}

	const XMLElement* root = _cache.RootElement();
	if (root == NULL)
	{
		return -1;
	}

	for (const XMLElement* currElem = root->FirstChildElement("File"); currElem != NULL; currElem = currElem->NextSiblingElement("File"))
	{
		if (currElem->Attribute("Path", path.c_str()) && currElem->Attribute("Configuration", configuration))
		{
			(*ppElem) = currElem;
			break;
		}
	}

	return ((*ppElem) == NULL);
}

string Cache::CalcHash(const char* code) const
{
	SHA3_512 sha;
	string out;
	Base64Encoder* enc = new Base64Encoder(new StringSink(out), false);

	StringSource(code, true, new HashFilter(sha, enc));

	return out;
}

string Cache::Normalize(const char* filePath) const
{
	size_t i;
	string path(filePath);

	while ((i = path.find('\\')) != string::npos)
	{
		path = path.replace(i, 1, "/");
	}
	while ((i = path.find("//")) != string::npos)
	{
		path = path.replace(i, 2, "/");
	}
	return path;
}

bool Cache::ReportCachedResults(const char* filePath, const char* configuration, const char* code, ErrorLogger* pLogger) const
{
	string cachedHash;
	size_t cachedSize = 0;
	const XMLElement* pElem = NULL;

	// Cached?
	if ((LoadFromCache(filePath, configuration, &cachedSize, &cachedHash, &pElem) != 0) || (cachedSize == 0) || cachedHash.empty() || (pElem == NULL))
	{
		return false;
	}

	// Compare size
	size_t actualSize = strlen(code);
	if (actualSize != cachedSize)
	{
		return false;
	}

	// Compare hash
	string actualHash = CalcHash(code);
	if (actualHash.compare(cachedHash) != 0)
	{
		return false;
	}

	for (const XMLElement* currChild = pElem->FirstChildElement("Report"); currChild != NULL; currChild = currChild->NextSiblingElement("Report"))
	{
		ErrorLogger::ErrorMessage msg;
		string t = currChild->GetText();
		if (msg.deserialize(t))
		{
			pLogger->reportErr(msg);
		}
	}

	return true;
}

int Cache::CacheFile(const char* filePath, const char* configuration, const char* code, const std::list<std::string>& reports)
{
	// Cache disabled
	if (_cacheFile.empty())
	{
		return 0;
	}

	string path = Normalize(filePath);
	if (path.empty())
	{
		return -1;
	}

	size_t actualSize = strlen(code);
	if (actualSize == 0)
	{
		return -1;
	}

	string actualHash = CalcHash(code);
	if (actualHash.empty())
	{
		return -1;
	}

	XMLElement* pElem = NULL;
	if ((Find(path.c_str(), configuration, &pElem) != 0) || (pElem == NULL))
	{
		// Create element
		XMLElement* root = _cache.RootElement();
		if (root == NULL)
		{
			root = _cache.NewElement("CppCheckCache");
			if (root == NULL)
			{
				return -1;
			}

			if (_cache.InsertFirstChild(root) == NULL)
			{
				return -1;
			}
		}

		pElem = _cache.NewElement("File");
		if (pElem == NULL)
		{
			return -1;
		}

		if (root->InsertEndChild(pElem) == NULL)
		{
			return -1;
		}
	}

	pElem->DeleteChildren();
	pElem->SetAttribute("Configuration", configuration);
	pElem->SetAttribute("Size", actualSize);
	pElem->SetAttribute("Hash", actualHash.c_str());
	pElem->SetAttribute("Path", path.c_str());

	// Insert reports
	std::list<std::string>::const_iterator currIt, endIt;
	for (currIt = reports.begin(), endIt = reports.end(); currIt != endIt; ++currIt)
	{
		XMLElement *report = _cache.NewElement("Report");
		if (report == NULL)
		{
			return -1;
		}

		report->SetText(currIt->c_str());
		pElem->InsertEndChild(report);
	}

	return 0;
}

int Cache::Remove(const char* filePath, const char* configuration)
{
	XMLElement* pElem = NULL;
	if ((Find(filePath, configuration, &pElem) != 0) || (pElem == NULL))
	{
		return 0;
	}

	XMLElement* root = _cache.RootElement();
	if (root == NULL)
	{
		return -1;
	}

	root->DeleteChild(pElem);
	return 0;
}

void Cache::Clear()
{
	_cache.Clear();
	_cacheFile.clear();
}

int Cache::Save()
{
	XMLError er = XML_SUCCESS;

	er = _cache.SaveFile(_cacheFile.c_str());

	return((er == XML_SUCCESS) || (er == XML_NO_ERROR)) ? 0 : (er | XML_SUCCESS);
}