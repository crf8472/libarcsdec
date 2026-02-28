#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#error "Do not include descriptor_details.hpp, include descriptor.hpp instead"
#endif
#ifndef __LIBARCSDEC_DESCRIPTOR_DETAILS_HPP__
#define __LIBARCSDEC_DESCRIPTOR_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of descriptor.hpp.
 */

#include <cctype>      // for toupper
#include <cstdint>     // for uint32_t, uint64_t, int64_t
#include <set>         // for set
#include <string>      // for basic_string, string, char_traits

namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{

class Bytes;

/**
 * \internal
 *
 * \brief Implementation details of namespace \c read.
 */
namespace details
{

/**
 * \brief Traits for case insensitive string comparison.
 *
 * Thanks to Herb Sutter: http://www.gotw.ca/gotw/029.htm
 */
struct ci_char_traits final : public std::char_traits<char>
{ // TODO const?
	static bool eq(char c1, char c2) { return toupper(c1) == toupper(c2); }

	static bool ne(char c1, char c2) { return toupper(c1) != toupper(c2); }

	static bool lt(char c1, char c2) { return toupper(c1)  < toupper(c2); }

	static int compare(const char* s1, const char* s2, size_t n)
	{
        while(n-- != 0)
		{
			if( toupper(*s1) < toupper(*s2) ) { return -1; }
			if( toupper(*s1) > toupper(*s2) ) { return  1; }

			++s1;
			++s2;
		}

		return 0;
	}

	static const char* find(const char* s, int n, char a)
	{
		while(n-- > 0 && toupper(*s) != toupper(a)) { ++s; }

		return s;
	}
};


/**
 * \brief Case insensitive comparable string.
 */
using ci_string = std::basic_string<char, ci_char_traits>;


/**
 * \brief Worker: default implementation for checking a filename.
 *
 * Returns TRUE if the suffix of the filename equals one of the internal
 * suffices. The check is done case-insensitive.
 *
 * \param[in] suffices Set of filename suffices
 * \param[in] filename Input filename to check
 *
 * \return TRUE if the filename suffix matches one of the internal suffices
 */
bool ci_match_suffix(const std::set<details::ci_string>& suffices,
		const std::string& filename);


/**
 * \brief Worker: Provides the suffix of a given filename.
 *
 * The suffix is the part of filename following the last occurrence of
 * \c delimiter. If filename does not contain the delimiter, the entire
 * filename is returned as suffix.
 *
 * \param[in] filename  The filename to check
 * \param[in] delimiter The delimiter to separate the suffix from the base
 *
 * \return The relevant suffix or the entire filename
 */
std::string get_suffix(const std::string& filename,
		const std::string& delimiter);

/**
 * \brief Worker: Read \c length bytes from file \c filename starting at
 * position \c offset.
 *
 * \param[in] filename Name of the file to read from
 * \param[in] offset   0-based byte offset to start
 * \param[in] length   Number of bytes to read
 *
 * \return Byte sequence read from file
 *
 * \throw FileReadException If the specified number of bytes could not be
 * read from the specified file and position
 *
 * \throw InputFormatException On unspecified error
 */
Bytes read_bytes(const std::string& filename,
	const uint32_t& offset, const uint32_t& length);

} // namespace details
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

