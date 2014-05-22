#ifndef BIN_REFLECTION_H
#define BIN_REFLECTION_H

#include <type_traits>

#include "reflection.h"
#include "bin_streams.h"

namespace bin_ser
{

//////////////////////////////////////////////////////////////////////////
// reflection processors
namespace details
{

struct write_processor
{
    explicit write_processor(output_stream& os)
        : os(os)
    {
    }

    template<class type>
    void operator()(char const *, type const& object) // just forwarding to outer functions for reflection
    {
        write(os, object);
    }

    output_stream& os;
};


struct read_processor
{
    explicit read_processor(input_stream& is)
        : is(is)
    {
    }

    template<class type>
    void operator()(char const *, type& object) // just forwarding to outer functions for reflection
    {
        read(is, object);
    }

    input_stream& is;
};

} // details

//////////////////////////////////////////////////////////////////////////
// common functions


inline void write(output_stream& os, bool object)
{
    uint32_t uobj = uint32_t(object);
    os.write(&uobj, sizeof(uobj));
}

inline void read(input_stream& is, bool& object)
{
    uint32_t  uobj;
    is.read(&uobj, sizeof(uobj));
    object = (uobj != 0);
}


// main functions - non-specialized types
template<class type>
void write( output_stream& os, type const& object,
            typename std::enable_if< ! boost::is_arithmetic<type>::value>::type* = 0)
{
    details::write_processor wp(os);
    reflect(wp, object);
}

template<class type>
void read(  input_stream& is, type& object,
            typename std::enable_if< ! boost::is_arithmetic<type>::value>::type* = 0)
{
    details::read_processor rp(is);
    reflect(rp, object);
}

// arithmetic types, using SFINAE
template<class type>
void write( output_stream& os, type object,
    typename std::enable_if<boost::is_arithmetic<type>::value>::type* = 0)
{
    static_assert(boost::is_arithmetic<type>::value, "type is arithmetic");
    os.write(&object, sizeof(object));
}

template<class type>
void read(  input_stream& is, type& object,
            typename std::enable_if<boost::is_arithmetic<type>::value>::type* = 0)
{
    static_assert(boost::is_arithmetic<type>::value, "type is arithmetic");
    is.read(&object, sizeof(object));
}

// some STL containers

// string
inline void write(output_stream& os, std::string const& str)
{
    os.write(str.c_str(), str.size() + 1);
}

inline void read(input_stream& is, std::string& str)
{
    str = is.curr();
    is.skip(str.size() + 1);
}

// vector<T, A>
template<class T, class A>
inline void write(output_stream& os, std::vector<T, A> const& vec)
{
    // could be optimized by direct writing in case of PODs
    write(os, uint32_t(vec.size()));
    for (size_t i = 0; i < vec.size(); ++i)
        write(os, vec[i]);
}

template<class T, class A>
inline void read(input_stream& is, std::vector<T, A>& vec)
{
    uint32_t size;
    read(is, size);

    vec.resize(size);
    for (size_t i = 0; i < size; ++i)
        read(is, vec[i]);
}

// map<K, V, C, A>
template <class K, class V, class C, class A>
inline void write(output_stream& os, std::map<K, V, C, A> const& m)
{
    write(os, uint32_t(m.size()));
    for (auto it = m.begin(); it != m.end(); ++it)
    {
        write(os, it->first);
        write(os, it->second);
    }
}

template <class K, class V, class C, class A>
inline void read(input_stream& is, std::map<K, V, C, A>& m)
{
    uint32_t size;
    read(is, size);

    for (size_t i = 0; i < size; ++i)
    {
        K key;

        read(is, key   );
        read(is, m[key]);
    }
}


//////////////////////////////////////////////////////////////////////////
// shorter way serialization

template<class type>
data_t wrap(type const& object)
{
    output_stream os;
    write(os, object);

    return os.raw_data();
}

template<class type>
void unwrap(data_cref data, type& object)
{
    input_stream is(&data[0], data.size());
    read(is, object);
}


} // bin_ser

#endif // BIN_REFLECTION_H
