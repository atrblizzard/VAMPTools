#pragma  once

#include <string>
#include <vector>

class BSPParser
{
public:

	// Extract the bps data from the file
	static bool ExtractData( class BSPFile *BSP );

private:

	// Extracts lump data and stores it in the BSP file
	static bool ExtractLumps( class BSPFile *BSP );

	// Extract entity data
	static bool ExtractEntities( class BSPFile *BSP, const struct lump_t& Lump );
	// Template function to extract data
	template<class T> static bool ExtractTypedData( class BSPFile *BSP, int TypeIdx, std::vector<T *>& lstData );
	// Extract game specific data from the game lump
	static bool ExtractGameLumpData( class BSPFile *BSP, const struct lump_t& Lump );
	// Extract string table data
	static bool ExtractStringTable( class BSPFile *BSP, const struct lump_t& Lump );
	// Extracts the pak file data
	static bool ExtractPakFiles( class BSPFile *BSP, const struct lump_t& Lump );

	// Returns the type of data based on the index
	static std::string GetType( int TypeIdx );
};