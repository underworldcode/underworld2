/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_FunctionIOClass_h__
#define __Underworld_Function_FunctionIOClass_h__

#include <vector>
#include <stddef.h>
#include <memory>
#include <typeindex>
#include <string>


#ifdef DEBUG
    #define debug_dynamic_cast dynamic_cast
#else
    #define debug_dynamic_cast static_cast
#endif

#if !defined(SWIG_DO_NOT_WRAP)
std::string functionio_get_type_name(std::type_index tindex);
#endif

class FunctionIO
{
    public:
        enum IOType
        {
            Scalar, Vector, SymmetricTensor, Tensor, Array
        };
        FunctionIO( IOType iotype ): _iotype(iotype), _dataType(typeid(NULL)){};
        virtual ~FunctionIO(){};
        virtual FunctionIO *clone() const = 0;
        IOType iotype() const {return _iotype;};
        virtual unsigned   size() const =0;
        virtual void resize(std::size_t vec_size) =0;
        template< typename T > T at( size_t idx=0 ) const { return at( (T) 0., idx ); };
        IOType _iotype;
        virtual       void* dataRaw()       =0;
        virtual const void* dataRaw() const =0;
#if !defined(SWIG_DO_NOT_WRAP)
        std::type_index dataType() const {return _dataType;};
        std::size_t _dataSize;
        std::type_index _dataType;
#endif
    protected:
        // Note here that the dummy arguments are included to facilitate overloading
        // as overloading based on return types alone is not possible.  Feels a little
        // bit dirty, but the standard does exactly this to allow overloading of the
        // postfix/prefix operators
        virtual const bool     at( bool     ignore, size_t idx ) const = 0;
        virtual const char     at( char     ignore, size_t idx ) const = 0;
        virtual const short    at( short    ignore, size_t idx ) const = 0;
        virtual const unsigned at( unsigned ignore, size_t idx ) const = 0;
        virtual const int      at( int      ignore, size_t idx ) const = 0;
        virtual const float    at( float    ignore, size_t idx ) const = 0;
        virtual const double   at( double   ignore, size_t idx ) const = 0;
};


// note we are using two parameters here because std::vector<bool> does not
// use native 'bool' types internally, and therefore we cannot use it in
// many of our algorithms which handle the memory directly.  Hence this
// double parameter template allows us to define FunctionIOTyped<bool> objects
// with std::vector<char> being the underlying data structure.
template< typename T, typename C >
class FunctionIOTyped : public FunctionIO
{
    public:
        FunctionIOTyped(std::size_t vec_size, FunctionIO::IOType iotype)
            : FunctionIO(iotype)
        {
            _vector.resize(vec_size);
            _dataSize = sizeof( T );
            _dataType = std::type_index(typeid(T));

        };
        virtual FunctionIOTyped<T,C> *clone() const { return new FunctionIOTyped<T,C>(*this); }
        virtual ~FunctionIOTyped(){};
        virtual unsigned size() const { return _vector.size(); };
        virtual void resize( std::size_t vec_size ) {
            _vector.resize(vec_size);
        };
        virtual       T* data()       { return reinterpret_cast<      T*>( _vector.data() ); };
        virtual const T* data() const { return reinterpret_cast<const T*>( _vector.data() ); };
        virtual       T& at(std::size_t idx=0)
        {
            #ifdef DEBUG
                return (T&)_vector.at(idx);
            #else
                return (T&)_vector[idx];
            #endif
        }
        virtual const T  at(std::size_t idx=0) const
        {
            #ifdef DEBUG
                return (const T)_vector.at(idx);
            #else
                return (const T)_vector[idx];
            #endif
        }

        void value(T val, std::size_t idx=0) { at(idx) = val;  };  // this guy is for python usage
        T    value(       std::size_t idx=0) { return at(idx); };  // this guy is for python usage
        virtual       void* dataRaw()       { return static_cast<      void*>( data() ); };
        virtual const void* dataRaw() const { return static_cast<const void*>( data() ); };

    protected:
        mutable std::vector<C> _vector;
        virtual const bool     at( bool     ignore, size_t idx ) const { return (bool    ) at(idx); };
        virtual const char     at( char     ignore, size_t idx ) const { return (char    ) at(idx); };
        virtual const short    at( short    ignore, size_t idx ) const { return (short   ) at(idx); };
        virtual const unsigned at( unsigned ignore, size_t idx ) const { return (unsigned) at(idx); };
        virtual const int      at( int      ignore, size_t idx ) const { return (int     ) at(idx); };
        virtual const float    at( float    ignore, size_t idx ) const { return (float   ) at(idx); };
        virtual const double   at( double   ignore, size_t idx ) const { return (double  ) at(idx); };
};


typedef FunctionIOTyped<bool,char>     IO_bool;
typedef FunctionIOTyped<char,char>     IO_char;
typedef FunctionIOTyped<short,short>    IO_short;
typedef FunctionIOTyped<unsigned,unsigned> IO_unsigned;
typedef FunctionIOTyped<int,int>      IO_int;
typedef FunctionIOTyped<float,float>    IO_float;
typedef FunctionIOTyped<double,double>   IO_double;




#endif /* __Underworld_Function_FunctionIOClass_h__ */

