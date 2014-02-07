#include <assert.h>
#include <memory>
#include <algorithm>

#include "ArgHandler.h"

using namespace std;

/** Sets an option and key value pair */
void ArgHandler::SetOpts( const char *Opt, const char *Key )
{
	string OptString = Opt;
	string KeyString = Key;

	if( OptString.empty() == false && KeyString.empty() == false )
	{
		// Make values lowercase!
		transform(OptString.begin(), OptString.end(), OptString.begin(),tolower);
		transform(KeyString.begin(), KeyString.end(), KeyString.begin(),tolower);
		
		// Store the option
		Opts[KeyString] = OptString;
	}
}

/** Parses the args */
void ArgHandler::EvalArgs( int NumArgs, char **Args )
{
	// Skip the first arg
	for( int Idx = 1; Idx < NumArgs; Idx++ )
	{
		string ArgString = Args[Idx];

		// Make Args lowercase!
		transform(ArgString.begin(), ArgString.end(), ArgString.begin(),tolower);
		
		// Iterate over the map
		bool bMatchedOpt = false;
		for( map<string,string>::iterator It = Opts.begin(); It != Opts.end(); ++It )
		{
			// Check to see if we found the option in the arg string
			basic_string<char>::size_type FindVal = ArgString.find((*It).second);
			if( FindVal != ArgString.npos )
			{
				Flags[(*It).first] = true;
				bMatchedOpt = true;
				break;
			}
		}

		// If we didn't match an option, add the argument to the list of general args
		if( bMatchedOpt == false )
		{
			GeneralArgs.push_back( ArgString );
		}
	}

#ifdef _DEBUG
	// Dump out the generic args
	printf( "(DEBUG) -- Non opt args:\n" );
	for( vector<string>::iterator It = GeneralArgs.begin(); It != GeneralArgs.end(); ++It )
	{
		printf( "  (DEBUG) -- %s\n", (*It).c_str() );
	}
	printf("\n");
#endif
}

/** Returns true if the specified key has been set */
bool ArgHandler::QueryKey( const char *Key )
{
	string KeyString = Key;

	map<string,bool>::iterator It = Flags.find(KeyString);
	if( It == Flags.end() )
	{
		return false;
	}

	return true;
}

/** Force a particular option value */
void ArgHandler::ForceOpt( const char *Key, bool Value )
{
	string KeyString = Key;

	if( KeyString.empty() == false )
	{
		Flags[KeyString] = Value;
	}
}

/** Returns TRUE if the argument is set correctly */
bool ArgHandler::GetArg( int Index, char **Arg )
{
	if( Index >= 0 && Index < NumArgs() )
	{
		if( GeneralArgs[Index].empty() == false )
		{
			(*Arg) = (char *)GeneralArgs[Index].c_str();
			return true;
		}
	}

	return false;
}

/** Returns the number of general arguments */
int ArgHandler::NumArgs()
{ 
	vector<string>::size_type Num;
	Num = GeneralArgs.size();
	return Num;
}
