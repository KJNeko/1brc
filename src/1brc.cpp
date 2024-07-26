#include <sys/mman.h>
#include <sys/stat.h>

#include <cassert>
#include <chrono>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ratio>
#include <string>
#include <unordered_map>

using namespace std;

#include "FGL_DEFINES.hpp"

/*

//TODO: Overwrite all the allocators but this probably is overkill
void* operator new( std::size_t count )
{
	auto ptr = malloc( count );
	return ptr;
}

void operator delete( void* ptr ) noexcept
{
	free( ptr );
}

void operator delete( void* ptr, [[maybe_unused]] std::size_t size ) noexcept
{
	free( ptr );
}
*/

// <mask, data>
// If the mask bit is 1 then it MUST be the data bit
constexpr std::array< std::tuple< std::uint8_t, std::uint8_t >, 4 > UTF8_BITMASKS {
	{ { std::uint8_t( 0b1000'0000 ), std::uint8_t( 0b0000'0000 ) },
	  { std::uint8_t( 0b1110'0000 ), std::uint8_t( 0b1100'0000 ) },
	  { std::uint8_t( 0b1111'0000 ), std::uint8_t( 0b1110'0000 ) },
	  { std::uint8_t( 0b1111'1000 ), std::uint8_t( 0b1111'0000 ) } }
};

consteval std::size_t getUTF8CharacterCount()
{
	std::size_t counter { 0 };
	for ( std::int8_t i = std::numeric_limits< std::int8_t >::min(); i < std::numeric_limits< std::int8_t >::max();
	      ++i )
	{
		for ( const auto& [ mask, data ] : UTF8_BITMASKS )
		{
			// If it matches a single mask then we will use it

			if ( ( i & mask ) == data )
			{
				++counter;
				break;
			}
		}
	}

	// Since the max number (255) isn't actually counted. We need to add 1 more to the final counter
	return counter + 1;
}

consteval std::array< std::uint8_t, getUTF8CharacterCount() > generateUTF8Characters()
{
	std::array< std::uint8_t, getUTF8CharacterCount() > array {};

	std::size_t counter { 0 };

	for ( std::int8_t i = std::numeric_limits< std::int8_t >::min(); i < std::numeric_limits< std::int8_t >::max();
	      ++i )
	{
		for ( const auto& [ mask, data ] : UTF8_BITMASKS )
		{
			// If it matches a single mask then we will use it

			if ( ( i & mask ) == data )
			{
				array[ counter ] = i;
				++counter;
				break;
			}
		}

		// Add the final max character if it will fit
		if ( counter + 1 < array.size() ) array[ counter + 1 ] = std::numeric_limits< std::int8_t >::max();
	}

	return array;
}

constexpr std::array< std::uint8_t, getUTF8CharacterCount() > UTF8_CHARACTERS { generateUTF8Characters() };



struct City
{
	std::string_view name;

	using minmaxType = std::int_fast16_t;
	using TempType = std::int_fast16_t;

	minmaxType min { std::numeric_limits< minmaxType >::max() };
	minmaxType max { std::numeric_limits< minmaxType >::min() };

	std::int32_t total { 0 };

	std::uint_fast32_t num { 0 };

	explicit City( const std::string_view city_name ) : name( city_name ) {}

	FGL_FLATTEN_HOT_INLINE void add_value( const TempType temp )
	{
		max = std::max( max, temp );
		min = std::min( min, temp );
		total += temp;
		num++;
	}

	// Not used yet but will be useful for dividing and conquering
	[[maybe_unused]] FGL_COLD void merge( const City& C )
	{
		max = std::max( max, C.max );
		min = std::min( min, C.min );
		total += C.total;
		num += C.num;
	}

	[[nodiscard]] float avg() const { return total / static_cast< float >( num ); }
};

struct Record
{
	std::string_view name {};
	float temp { 0.0 };
};

struct Bucket
{
	unordered_map< std::string_view, City > bucket {};

	Bucket()
	{
		// At most we will expect 10,000 unique city names
		bucket.reserve( 10'000 );
	}

	FGL_FLATTEN_HOT_INLINE void parse_record( const Record& R )
	{
		if ( const auto itter = bucket.find( R.name ); itter == bucket.end() )
		{
			City C { R.name };
			C.add_value( R.temp );
			bucket.insert( { R.name, C } );
		}
		else
		{
			itter->second.add_value( R.temp );
		}
	}

	// Not used yet but will be useful for dividing and conquering
	[[maybe_unused]] FGL_COLD void merge( const Bucket& B )
	{
		for ( const auto& [ name, C ] : B.bucket )
		{
			if ( auto itter = bucket.find( name ); itter == bucket.end() )
			{
				bucket.insert( { name, C } );
			}
			else
			{
				itter->second.merge( C );
			}
		}
	}

	// Gonna want to implement this if you want to actually check for correctness
	// and compare with reference scripts
	[[maybe_unused]] void print() { return; }
};

FGL_FLATTEN_HOT_INLINE inline Record split( const std::string_view& line )
{
	Record R;

	const std::string::size_type i { line.find_first_of( ';' ) };

	// This might be off by 1
	R.name = line.substr( 0, i );

	std::from_chars( line.data() + i + 1, line.data() + line.length() - i + 1, R.temp );

	return R;
}

constexpr std::string_view path { "./1brc/measurements.txt" };

struct ViewBuffer : std::streambuf
{
	ViewBuffer( const char* data, const std::size_t size )
	{
		// Interface requires it to be non const but won't date change the data. So const cast is 'safe' here.
		std::streambuf::char_type* ptr { const_cast< char* >( data ) };
		this->setg( ptr, ptr, ptr + size );
	}
};

struct MemoryStream final : ViewBuffer, std::istream
{
	MemoryStream( const char* data, const std::size_t size ) :
	  ViewBuffer( data, size ),
	  std::istream( static_cast< std::streambuf* >( this ) )
	{}

	MemoryStream( const std::string_view& view ) : MemoryStream( view.data(), view.size() ) {}
};

int main()
{
	const auto start { chrono::steady_clock::now() };

	// this could be an input parameter but I'm lazy
	// the reference scripts all by default hardcoded to read from "measurements.txt"
	const std::filesystem::path filename { path };

	assert( std::filesystem::exists( filename ) );

	const std::uint64_t filesize { std::filesystem::file_size( filename ) };

	// std::byte* ptr { new std::byte[ filesize ] };

	const auto fd { open( filename.c_str(), O_RDONLY ) };

	void* file_ptr { mmap( nullptr, filesize, PROT_READ, MAP_SHARED, fd, 0 ) };

	const std::string_view data { static_cast< std::string_view::value_type* >( file_ptr ), filesize };

	string line;
	std::uint_fast32_t N { 0 };

	Bucket B {};

	MemoryStream ss { data };

	while ( std::getline( ss, line ) )
	{
		N++;

		Record R = split( line );
		B.parse_record( R );

		if ( N % 100000 == 0 )
		{
			cout << N << "\r" << flush;
		}
	}

	cout << "\n";

	auto end = chrono::steady_clock::now();
	chrono::duration< float, milli > dur = end - start;

	cout << dur.count() << "ms\n";

	return EXIT_SUCCESS;
}
