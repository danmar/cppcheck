
//---------------------------------------------------------------------------
#ifndef tempcacheH
#define tempcacheH
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
 * @brief		Class maintains a temporary cache for a single file and single configuration
 *
 * @details		A scope-based temporay cache to accumulate check results before dumping them all to global cache.
 */
class CPPCHECKLIB TempCache {
public:

	/*! @brief		Constructor
	*/
	TempCache(ErrorLogger* logger, Settings *settings, const std::string& filename, const std::string& cfg, const std::string& codeWithoutCfg)
		: _settings(settings)
		, _logger(logger)
		, _filename(filename)
		, _cfg(cfg)
		, _codeWithoutCfg(codeWithoutCfg)
	{}
	
	/*! @brief		Dump errors to global cache
	*/
	~TempCache()
	{
		if (0 > _settings->cache.CacheFile(_filename.c_str(), _cfg.c_str(), _codeWithoutCfg.c_str(), _reportCache))
		{
			_logger->reportOut("Failed caching file: " + _filename);
		}
	}
	
	/*! @brief		Report an error
	*/
	void Report(const std::string& msg)
	{
		_reportCache.push_back(msg);
	}

private:

	Settings *_settings;
	ErrorLogger* _logger;
	std::string _filename, _cfg, _codeWithoutCfg;
	std::list<std::string> _reportCache;
};

/// @}
//---------------------------------------------------------------------------
#endif //  tempcacheH
