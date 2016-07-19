
//---------------------------------------------------------------------------
#ifndef cacheH
#define cacheH
//---------------------------------------------------------------------------

#include "config.h"
#include <string>
#include "tinyxml2.h"
#include <list>
#include <string>
class ErrorLogger;
/// @addtogroup Core
/// @{

/**
 * @brief Class maintains a cache of checked files
 * 
 * Cache is enabled by command line switch --cache=_CacheFile_
 */
class CPPCHECKLIB Cache {
public:

	/*! @brief		Constructor
	*/
	Cache();

	/*! @brief		Copy Constructor
	*/
	Cache(const Cache& other);

	/*!	@brief		Copy another Cache instance
		@param		other- Cache to copy
		@return		Reference to this instance.
	*/
	Cache& operator=(const Cache& other);

	/*!	@brief		Loads cache from file
		@param		cacheFile- File used to load and save cached files data
		@return		Zero if file was successfully loaded to cache
		*/
	int Load(const std::string& cacheFile);

	/*! @brief		Load cached results and report them
		@details	If a file has not changed since cached, load cached results and report them
		@param		filePath- Path to the file in question
		@param		configuration- Preprocessor configuration
		@param		code- Preprocessor code, as returned from Preprocessor::getcode()
		@param		pLogger- Logger to report errors to
		@return		true if and only if the file is in cache and has not changed since cached.
	*/
	bool ReportCachedResults(const char* filePath, const char* configuration, const char* code, ErrorLogger* pLogger) const;

	/*!	@brief		Place a file in cache
		@details	Caches the requested file in cache along with its hash value and size. 
					If a file is present in cache it is replaced with the new hash and size.
		@param		filePath- Path to the file to store in cache
		@param		configuration- Preprocessor configuration
		@param		code- Preprocessor code, as returned from Preprocessor::getcode()
		@return		Zero if file was successfully cached
	*/
	int CacheFile(const char* filePath, const char* configuration, const char* code, const std::list<std::string>& reports);

	/*!	@brief		Removes a file from cache
		@details	Removes the requested file from cache.
		@param		filePath- Path to the file to store in cache
		@param		configuration- Preprocessor configuration
		@return		Zero if file was successfully removed from cache
	*/
	int Remove(const char* filePath, const char* configuration);

	/*!	@brief		Clears the cache
		@details	Clear the cache entirely, without saving to file
	*/
	void Clear();

	/*!	@brief		Saves the cache to file
		@details	All operations since constructing the cache are performed in memory. Only when Save() is explicitly called will the cache be persisted to file.
		@return		Zero if cache was successfully saved
	*/
	int Save();

protected:

	/*!	@brief		Calaculate a file hash
		@details	For a given file path, calculate the file hash. Currently we use MD5 hash
		@param		code- Preprocessor code, as returned from Preprocessor::getcode()
		@return		Hash, or empty string if an error occured
	*/
	virtual std::string CalcHash(const char* code) const;

	/*! @brief		Loads a file data from cache
		@details	Checks if the requested file is present in cache. If yes, output its cached size and hash
		@param		filePath- Path to the file in question
		@param		configuration- Preprocessor configuration
		@param[out]	pCachedSize- Pointer to a size_t variable to receive the file's cached size, if available
		@param[out]	pHash- Pointer to a string variable to receive the file's cached hash value, if available
		@param[out]	ppElem- Pointer to a XML element to receive the cached element, if available
		@return		Zero if file was successfully retreived from cache
	*/
	virtual int LoadFromCache(const char* filePath, const char* configuration, size_t* pCachedSize, std::string* pHash, const tinyxml2::XMLElement** ppElem) const;

	/*!	@brief		Returns a normalized representation of the file path
		@details	As file path separators are file system dependant, this function ensures a distinct represenatation accross file systems. 
					In this implementation, '\' is replaced with '/', and '//' is replaced with '/'
		@param		filePath- Path to normalize
		@return		A normalized form of path
	*/
	virtual std::string Normalize(const char* filePath) const;

private:

	int Find(const char* filePath, const char* configuration, const tinyxml2::XMLElement** ppElem) const;
	int Find(const char* filePath, const char* configuration, tinyxml2::XMLElement** ppElem);

	tinyxml2::XMLDocument _cache;	
	std::string _cacheFile;
};

/// @}
//---------------------------------------------------------------------------
#endif //  cacheH
