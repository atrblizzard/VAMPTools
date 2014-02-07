#ifndef __MDLPARSER__
#define __MDLPARSER__

#include <string>
#include <vector>

// Also defined in studio.h
#define NUMANIMCHANNELS 7

#define BONEFLAG_ORIENTATION 0x2

//class MDLFile
class MDLParser
{
public:

	// Extract the model data from the file
	static bool ExtractData( class MDLFile *MDL );

private:
	
	// Extracts texture data and stores it in the MDLFile
	static bool ExtractTextures( class MDLFile *MDL );
	// Extracts material data and stores it in the MDLFile
	static bool ExtractMaterials( class MDLFile *MDL, const std::vector<std::string>& SearchPaths );
	// Extracts bone data and stores it in the MDLFile
	static bool ExtractSkeleton( class MDLFile *MDL );
	// Extracts animation data and stores it in the MDLFile
	static bool ExtractAnimations( class MDLFile *MDL );

	// Recursively fetches all of the include models
	static void GetIncludedModels( class MDLFile *MDL, std::vector<class MDLFile *> &IncludedModels );

	static std::vector<std::string> BonesToPrune;
	static std::vector<std::string> SearchPaths;
};

#endif // __MDLPARSER__