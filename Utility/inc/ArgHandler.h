#ifndef __ARGHANDLER__
#define __ARGHANDLER__

#include <string>
#include <map>
#include <vector>

class ArgHandler
{
public:

	ArgHandler() {}

	~ArgHandler() {}

	// Specify an option (like "-?") and an associated key (like "help") which can be queried later
	void SetOpts( const char* Opt, const char* Key );

	// Evaluate the arguments passed in
	void EvalArgs( int NumArgs, char **Args );

	// Returns TRUE if the provided option was specified
	bool QueryKey( const char *Key );

	// Forces the specified option
	void ForceOpt( const char *Key, bool Value );

	// Returns TRUE if an argument param is filled
	bool GetArg( int Index, char **Arg );

	// Returns the number of general arguments
	int NumArgs();

private:

	std::map<std::string, std::string> Opts;
	std::map<std::string, bool> Flags;

	// General arguments
	std::vector<std::string> GeneralArgs;
};

#endif // __ARGHANDLER__