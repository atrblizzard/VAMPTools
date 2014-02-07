#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>

class Utility
{
public:
	static void FixupSlashes( std::string& Path );
	static void FixupShaderValue( std::string& Value );
	static void FixupBoneName( std::string& BoneName );
	static void FixupModelName( std::string& ModelName );
	static void FindAndReplace( std::string& Source, const std::string& Search, const std::string& Replace );

	static void ParseIntoArray( const std::string& Source, char Delim, std::vector<std::string>& Array );

	static void CleanupQuotes( std::string& Source );

	static bool FindVMTFile( const std::string& TextureName, const std::string& BaseFilename, const std::vector<std::string>& BaseSearchPaths, std::string &VMTFilePath );
	static bool FindTTHFile( const std::string& TextureName, const std::string& BaseFilename, const std::vector<std::string>& BaseSearchPaths, const std::vector<std::string>& TempSearchPaths, std::string &TTHFilePath );
	static bool FindTTZFile( const std::string& TextureName, const std::string& BaseFilename, const std::vector<std::string>& BaseSearchPaths, const std::vector<std::string>& TempSearchPaths, std::string &TTZFilePath );
	static bool FindTextureFile( const std::string &TextureName, const std::string& BaseFilename, const std::vector<std::string>& BaseSearchPaths, const std::vector<std::string>& TempSearchPaths, const std::string &Ext, std::string &OutFilePath );

	static bool ContainsStr( std::vector<std::string>& InVec, const std::string& InStr );

	static bool FindModelFile( const std::string& BaseFilename, const std::string& Filename, std::string& FullFilePath );
	static unsigned int GetFileSize( FILE *F );

	static bool GetFilename( const std::string& Path, std::string& OutFilename );
	static bool GetBaseFilePath( const std::string& Path, std::string& OutPath );
	static void StripExtension( std::string& Filename );

	static void MakeUtilityPath( const std::string& BaseFilename, const std::string& FileExt, std::string& OutputPath );
	static void MakeStaticPropListPath( const std::string& BaseFilename, std::string& OutputPath );
	static void MakePakfilePath( const std::string& BaseFilename, std::string& OutputPath );
	static void MakeTextureListPath( const std::string& BaseFilename, std::string& OutputPath );
	static void MakeTextureOutputPath( const std::string& BaseFilename, const std::string& TextureName, std::string& OutputPath );
	static void MakeAnimOutputPath( const std::string& BaseFilename, const std::string& AnimName, std::string& OutputPath );

	static bool IsPruneBone( const std::string &BoneName );

	static std::string MDLExt;
	static std::string VTXExt;
	static std::string VTXExt2;
	static std::string VMTExt;
	static std::string TTHExt;
	static std::string TTZExt;
	static std::string TGAExt;
	static std::string FBXExt;
	static std::string BSPExt;
};

#endif // UTILITY_H