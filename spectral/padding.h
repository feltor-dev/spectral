#ifndef _TL_PADDING_
#define _TL_PADDING_

namespace spectral{

/*! @brief Provide various padding types of the Matrix class
 * @ingroup containers
 */
enum Padding{ TL_NONE, //!< Don't use any padding
              TL_DFT, //!< Pad lines with 2-cols%2 elements for inplace DFT in horizontal direction.
              TL_DRT_DFT //!< Add two 2-rows%2 lines at end of matrix for inplace DFT in vertical direction
            };

///@cond

/*! @brief template traits class for the efficient implementation of
 * the access operators in the matrix class.
 *
 * @containers
 * These functions are also used in copy and construction operators.
 */
template <enum Padding P> //i.e. TL_NONE
struct TotalNumberOf
{
    /*! @brief Return # of columns including padded ones
     *
     * @param cols # of visible columns in the matrix
     * @return Total # of columns in the matrix
     */
    static inline size_t columns( const size_t cols){return cols;}
    /*! @brief Return total # of elements in the matrix
     *
     * @param rows # of visible rows in the matrix
     * @param cols # of visible columns in the matrix
     * @return Total # of elements in the matrix
     */
    static inline size_t elements( const size_t rows, const size_t cols){return rows*cols;}
};

template <>
struct TotalNumberOf<TL_DFT>
{
    static inline size_t columns( const size_t m){ return m - m%2 + 2;}
    static inline size_t elements( const size_t n, const size_t m){return n*(m - m%2 + 2);}
};

template <>
struct TotalNumberOf<TL_DRT_DFT>
{
    static inline size_t columns( const size_t m){ return m;}
    static inline size_t elements( const size_t n, const size_t m){return m*(n - n%2 + 2);}
};
///@endcond



} //namespace spectral



#endif //_TL_PADDING_
