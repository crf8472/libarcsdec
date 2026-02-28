#ifndef __LIBARCSDEC_METAPARSER_HPP__
#error "Do not include metaparser_details.hpp, include metaparser.hpp instead"
#endif
#ifndef __LIBARCSDEC_METAPARSER_DETAILS_HPP__
#define __LIBARCSDEC_METAPARSER_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of metaparser.hpp.
 */

#include <limits>       // for numeric_limits
#include <ostream>      // for ostringstream
#include <stdexcept>    // for out_of_range, runtime_error
#include <type_traits>  // for is_signed, is_unsigned

namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{
namespace details
{

/**
 * \brief Returns TRUE if types S and T are either both signed or both unsigned.
 *
 * \tparam S Left type
 * \tparam T Right type
 */
template <typename S, typename T>
struct signedness final : public std::integral_constant<bool,
	(std::is_signed<S>::value && std::is_signed<T>::value)
	|| (std::is_unsigned<S>::value && std::is_unsigned<T>::value)>
{
	// empty
};


/**
 * \brief Service method: Cast a value of some integral type safely to an
 * integral type of smaller range.
 *
 * The types must either both be signed or both be unsigned.
 *
 * If the input type is within the range of the target type, the cast is
 * performed, otherwise an exception is thrown.
 *
 * \param[in] value The value to convert
 *
 * \throw out_of_range If \c value is out of the range of target type
 *
 * \return The numerical value
 */
template <typename S, typename T,
		 std::enable_if_t<details::signedness<S, T>::value, int> = 0>
inline auto cast_or_throw(const T value) -> S
{
	auto throw_with_message = [](const T val, const std::string& msg)
	{
		std::ostringstream stream;
		stream << "Value " << val << " " << msg;
		throw std::out_of_range(stream.str());
	};

	if (value < std::numeric_limits<S>::min())
	{
		throw_with_message(value, "is too small to cast");
	}

	if (value > std::numeric_limits<S>::max())
	{
		throw_with_message(value, "is too big to cast");
	}

	return static_cast<S>(value);
}


/**
 * \brief Convert MSF time to CDDA frames.
 *
 * \param[in] m Minutes
 * \param[in] s Seconds
 * \param[in] f Frames
 *
 * \return Total number of CDDA frames
 */
long msf_to_frames(const int m, const int s, const int f);


/**
 * \brief Convert CDDA frames to MSF.
 *
 * \param[in]  frames Total number of CDDA frames to convert
 * \param[out] m      Minutes
 * \param[out] s      Seconds
 * \param[out] f      Frames
 */
void frames_to_msf(long frames, int* m, int* s, int* f);


} // namespace details
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

