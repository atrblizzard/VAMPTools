#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <io.h>

#include "Utility.h"

using namespace std;

string Utility::MDLExt = ".mdl";
string Utility::VTXExt = ".dx80.vtx";
string Utility::VTXExt2 = ".dx7_2bone.vtx";
string Utility::VMTExt = ".vmt";
string Utility::TTHExt = ".tth";
string Utility::TTZExt = ".ttz";
string Utility::TGAExt = ".tga";
string Utility::FBXExt = ".fbx";
string Utility::BSPExt = ".bsp";

static std::vector<std::string> DefaultTexturePaths;
static std::vector<std::string> PruneBones;

void InitializeDefaultPaths()
{
	DefaultTexturePaths.clear();
	DefaultTexturePaths.push_back( "models\\character\\" );
	DefaultTexturePaths.push_back( "models\\character\\eyes\\" );
}

void InitializePruneBones()
{
	PruneBones.clear();
	PruneBones.push_back( "Sledgehammer" );
	PruneBones.push_back( "sledgehammer" );
	PruneBones.push_back( "Bat" );
	PruneBones.push_back( "bat" );
	PruneBones.push_back( "Handle" );
	PruneBones.push_back( "handle" );
	PruneBones.push_back( "Gerber" );
	PruneBones.push_back( "gerber" );
	PruneBones.push_back( "Cylinder01" );
	PruneBones.push_back( "tire_iron" );

	// Static props shouldn't have a skeleton
	PruneBones.push_back( "static_prop" );
}

void Utility::FixupSlashes( string& Path )
{
	FindAndReplace( Path, "/", "\\" );
}

void Utility::FixupShaderValue( string &Value )
{
	transform( Value.begin(), Value.end(), Value.begin(), tolower );
	FixupSlashes( Value );
	FindAndReplace( Value, "\"", "" );
}

void Utility::FixupBoneName( string &BoneName )
{
	// Bone names shouldn't have spaces in them
	FindAndReplace( BoneName, " ", "_" );
}

void Utility::FixupModelName( string& ModelName )
{
	// Find the last _ in the string
	int nLoc = ModelName.find_last_of( "_" );
	if( nLoc != string::npos )
	{
		// Strip out everything from that point in the string on
		ModelName.erase( nLoc );
	}
}

void Utility::FindAndReplace( string& Source, const string& Search, const string& Replace )
{
	int nPos = 0;
	
	nPos = Source.find(Search,nPos);
	while( nPos != string::npos )
	{
		Source.replace( nPos, Search.length(), Replace );
		nPos += Replace.length();

		// Look for the next instance of search
		nPos = Source.find( Search, nPos );
	}
}

/** Parses the buffer into some arrays based on the delimiter */
void Utility::ParseIntoArray( const string& Source, char Delim, vector<string>& Array )
{
	stringstream Stream( Source );
	string Item;

	while( getline(Stream,Item,Delim) )
	{
		Array.push_back( Item );
	}
}

void Utility::CleanupQuotes( string& Source )
{
	// Kill the trailing quotation mark and space
	FindAndReplace( Source, "\" ", "" );

	// Find the last quotation mark and nuke it
	int nPos = Source.find_last_of( "\"" );
	if( nPos != string::npos )
	{
		Source.erase( nPos, 1 );
	}

	// Find the first quotation mark and nuke it
	nPos = Source.find_first_of( "\"" );
	if( nPos != string::npos )
	{
		Source.erase( nPos, 1 );
	}
}

bool Utility::FindVMTFile( const string& TextureName, const string& BaseFilename, const vector<string>& BaseSearchPaths, string &VMTFilePath )
{
	vector<string> Empty;
	return FindTextureFile( TextureName, BaseFilename, BaseSearchPaths, Empty, VMTExt, VMTFilePath );
}

bool Utility::FindTTHFile( const string& TextureName, const string& BaseFilename, const vector<string>& BaseSearchPaths, const vector<string>& TempSearchPaths, string &TTHFilePath )
{
	return FindTextureFile( TextureName, BaseFilename, BaseSearchPaths, TempSearchPaths, TTHExt, TTHFilePath );
}

bool Utility::FindTTZFile( const string& TextureName, const string& BaseFilename, const vector<string>& BaseSearchPaths, const vector<string>& TempSearchPaths, string &TTZFilePath )
{
	return FindTextureFile( TextureName, BaseFilename, BaseSearchPaths, TempSearchPaths, TTZExt, TTZFilePath );
}

bool Utility::FindTextureFile( const string &TextureName, const string& BaseFilename, const vector<string>& BaseSearchPaths, const vector<string>& TempSearchPaths, const string &Ext, string &OutFilePath )
{
	// Construct a base path
	string BasePath = BaseFilename;
	int nPos = BasePath.find_last_of( "\\" );
	BasePath.erase( nPos + 1 );

	// Make the default base filename
	string BaseTexName = TextureName + Ext;
	// Make sure slashes are correct
	FixupSlashes( BaseTexName );

	// First try the binary directory
	if( _access(BaseTexName.c_str(),0) == 0 )
	{
		OutFilePath = BaseTexName;
		return true;
	}

	// Next try the same directory as the input filename
	string TestPath = BasePath + BaseTexName;
	if( _access(TestPath.c_str(),0) == 0 )
	{
		OutFilePath = TestPath;
		return true;
	}

	// Now we get tricky... try to recurse up the path to see if we can find the vampire directory
	nPos = BasePath.find( "Vampire" );
	if( nPos == string::npos )
	{
		nPos = BasePath.find( "vampire\\" );
	}

	// If we found something...
	if( nPos != string::npos )
	{
		string VampBasePath = BasePath;
		// Get the vampire directory with the trailing slash
		VampBasePath.erase( nPos + 8 );

		// Add the base materials directory
		VampBasePath += "materials\\";

		// Try the temp search paths first
		for( vector<string>::size_type Idx = 0; Idx < TempSearchPaths.size(); Idx++ )
		{
			TestPath = VampBasePath + TempSearchPaths[Idx] + BaseTexName;
			if( _access(TestPath.c_str(),0) == 0 )
			{
				OutFilePath = TestPath;
				return true;
			}
		}

		// Try the base search paths
		for( vector<string>::size_type Idx = 0; Idx < BaseSearchPaths.size(); Idx++ )
		{
			TestPath = VampBasePath + BaseSearchPaths[Idx] + BaseTexName;
			if( _access(TestPath.c_str(),0) == 0 )
			{
				OutFilePath = TestPath;
				return true;
			}
		}

		// Try the default search paths
		for( vector<string>::size_type Idx = 0; Idx < DefaultTexturePaths.size(); Idx++ )
		{
			TestPath = VampBasePath + DefaultTexturePaths[Idx] + BaseTexName;
			if( _access(TestPath.c_str(),0) == 0 )
			{
				OutFilePath = TestPath;
				return true;
			}
		}
	}

	return false;
}

/** Returns TRUE if the specified string is in the string vector */
bool Utility::ContainsStr( vector<string>& InVec, const std::string& InStr )
{
	for( vector<string>::iterator It = InVec.begin(); It != InVec.end(); ++It )
	{
		int nPos = InStr.find( (*It) );
		if ( nPos != string::npos )
		{
			return true;
		}
	}

	return false;
}

/** Searches for the specified filename in various directories */
bool Utility::FindModelFile( const string& BaseFilename, const string& Filename, string& FullFilePath )
{
	string BasePath = BaseFilename;
	string Path = Filename;

	// Adjust slashes
	FixupSlashes( Path );

	// Try to recurse up the path to see if we can find the 'vampire' directory
	int nPos = BasePath.find( "Vampire" );
	if( nPos == string::npos )
	{
		nPos = BasePath.find( "vampire\\" );
	}

	// If we found something...
	if( nPos != string::npos )
	{
		string VampBasePath = BasePath;
		// Get the vampire directory with the trailing slash
		VampBasePath.erase( nPos + 8 );

		VampBasePath += Path;
		if( _access(VampBasePath.c_str(),0) == 0 )
		{
			FullFilePath = VampBasePath;
			return true;
		}
	}

	return false;
}

/** Returns the current file position */
unsigned int Utility::GetFileSize( FILE *F )
{
	if( F )
	{
		unsigned int OldFilePos = ftell( F );

		fseek( F, 0, SEEK_END );
		unsigned int Size = ftell( F );
		fseek( F, OldFilePos, SEEK_SET );

		return Size;
	}

	return 0;
}

/** Returns the file name (given a full path to a file) */
bool Utility::GetFilename( const string& Path, string& OutFilename )
{
	int nPos = Path.find_last_of( "\\" );
	if( nPos != string::npos )
	{
		OutFilename = Path;
		OutFilename.erase( 0, nPos + 1 );
		return true;
	}

	return false;
}

/** Returns the file path (given a full path to a file) */
bool Utility::GetBaseFilePath( const string& Path, string& OutPath )
{
	int nPos = Path.find_last_of( "\\" );
	if( nPos != string::npos )
	{
		OutPath = Path;
		OutPath.erase( nPos + 1 );
		return true;
	}

	return false;
}

/** Strips the file extension */
void Utility::StripExtension( string& Filename )
{
	int nPos = Filename.find_last_of( "." );
	if( nPos != string::npos )
	{
		Filename.erase( nPos );
	}
}

/** Creates a utility path with the specified extension */
void Utility::MakeUtilityPath( const std::string& BaseFilename, const std::string& FileExt, std::string& OutputPath )
{
	if( GetBaseFilePath(BaseFilename,OutputPath) )
	{
		string Filename;
		GetFilename(BaseFilename,Filename);
		StripExtension(Filename);

		OutputPath += Filename + FileExt;
	}
}

/** Create static prop output path */
void Utility::MakeStaticPropListPath( const string& BaseFilename, string& OutputPath )
{
	MakeUtilityPath( BaseFilename, "_sprp.txt", OutputPath );
}

/** Creates a pakfile output path */
void Utility::MakePakfilePath( const string& BaseFilename, string& OutputPath )
{
	MakeUtilityPath( BaseFilename, "_pak.zip", OutputPath );
}

/** Creates an output path for the texture list */
void Utility::MakeTextureListPath( const string& BaseFilename, string& OutputPath )
{
	MakeUtilityPath( BaseFilename, "_TexList.txt", OutputPath );
}

/** Creates an output path at the same place as the input model filename */
void Utility::MakeTextureOutputPath( const string& BaseFilename, const string& TextureName, string& OutputPath )
{
	// Get the base file path
	if( GetBaseFilePath(BaseFilename,OutputPath) )
	{
		OutputPath += TextureName + TGAExt;
	}
}

/** Creates an output path for the FBX file at the same place as the input model filename */
void Utility::MakeAnimOutputPath( const string& BaseFilename, const string& AnimName, string& OutputPath )
{
	// Get the base file path
	if( GetBaseFilePath(BaseFilename,OutputPath) )
	{
		OutputPath += AnimName + FBXExt;
	}
}

/** Returns true if the specified bone is in the prune bones list */
bool Utility::IsPruneBone( const std::string &BoneName )
{
	for( vector<string>::iterator It = PruneBones.begin(); It != PruneBones.end(); ++It )
	{
		int nPos = BoneName.find( (*It) );
		if ( nPos != string::npos )
		{
			return true;
		}
	}

	return false;
}