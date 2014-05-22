#ifndef PTREE_REFLECTION_H
#define PTREE_REFLECTION_H
#pragma once

#include <type_traits>
#include <boost/property_tree/ptree.hpp>

#include "reflection.h"

namespace ptree_ser
{

using namespace boost::property_tree;


//
namespace details
{

// type traits
template<class T, class S>
struct is_equal_type
{
    static const bool value = false;
};

template<class T>
struct is_equal_type<T, T>
{
    static const bool value = true;
};

template<class T>
struct is_leaf_type
{
    enum { value = is_equal_type<T, std::string>::value || boost::is_arithmetic<T>::value };
};

struct write_processor;
struct read_processor;

} // details

// forwards
template<class T, class A>
void write(ptree& pt, std::vector<T, A> const& vector_value);
template<class T, class A>
void read(ptree const& tree, std::vector<T, A>& vector_value);

template <class K, class V, class C, class A>
void write(ptree& pt, std::map<K, V, C, A> const& map_value);
template <class K, class V, class C, class A>
void read(ptree const& tree, std::map<K, V, C, A>& map_value);

// general functions
template<class type>
void write(ptree& pt, type const& object);
template<class type>
void read(ptree const& pt, type& object);

//////////////////////////////////////////////////////////////////////////
// common functions

// main functions - non-specialized types
template<class type>
void write( ptree& pt, string const& key, type const& value,
            typename std::enable_if< ! details::is_leaf_type<type>::value>::type* = 0)
{
    write(pt.put_child(ptree::path_type(key, '|'), ptree()), value);
}

template<class type>
void read(  ptree const& pt, string const& key, type& value,
            typename std::enable_if< ! details::is_leaf_type<type>::value>::type* = 0)
{
    read(pt.get_child(key), value);
}


// arithmetic types, using SFINAE
template<class type>
void write( ptree& pt, string const& key, type const& value,
            typename std::enable_if<details::is_leaf_type<type>::value>::type* = 0)
{
    pt.put(ptree::path_type(key, '|'), value);
}

template<class type>
void read(  ptree const& pt, string const& key, type& value,
            typename std::enable_if<details::is_leaf_type<type>::value>::type* = 0)
{
    value = pt.get<type>(ptree::path_type(key, '|'));
}

// stl containers (you could add you own, if needed)

// vector
template<class T, class A>
void write(ptree& pt, std::vector<T, A> const& vector_value)
{
    for (size_t i = 0; i < vector_value.size(); ++i)
        write(pt, boost::lexical_cast<string>(i), vector_value[i]);
}

template<class T, class A>
void read(ptree const& tree, std::vector<T, A>& vector_value)
{
    vector_value.resize(tree.size());

    size_t i = 0;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        read(tree, boost::lexical_cast<string>(i), vector_value[i]);
        ++i;
    }
}

// map
template <class K, class V, class C, class A>
void write(ptree& pt, std::map<K, V, C, A> const& map_value)
{
    for (auto it = map_value.begin(); it != map_value.end(); ++it)
        write(pt, lexical_cast<string>(it->first), it->second);
}

template <class K, class V, class C, class A>
void read(ptree const& tree, std::map<K, V, C, A>& map_value)
{
    for (auto it = tree.begin(); it != tree.end(); ++it)
        read(tree, it->first, map_value[lexical_cast<K>(it->first)]);
}



// general functions
template<class type>
void write(ptree& pt, type const& object)
{
    details::write_processor wp(pt);
    reflect(wp, object);
}

template<class type>
void read(ptree const& pt, type& object)
{
    details::read_processor rp(pt);
    reflect(rp, object);
}


//////////////////////////////////////////////////////////////////////////
// short way for streams operations
enum parser_t { INFO, JSON, XML };

template<class T,parser_t p>
struct io_parser {

    template< parser_t parser_type > struct parser_type_ { enum { value = parser_type}; };

    typedef parser_type_<INFO>  info_;
    typedef parser_type_<JSON> json_;
    typedef parser_type_<XML>   xml_;

    static inline void read_from(std::istream& is, T& value)
    {
        read_from(is,value,parser_type_<p>());
    }

    static inline void write_to(std::ostream& os, T const& value)
    {
        write_to(os,value,parser_type_<p>());
    }

    static inline void read_from(std::istream& is, T& value, info_)
    {
        ptree tree;
        info_parser::read_info(is, tree);

        read(tree, value);
    }

    static inline void write_to(std::ostream& os, T const& value, info_)
    {
        ptree tree;
        write(tree, value);

        info_parser::write_info(os, tree);
    }

    static inline void read_from(std::istream& is, T& value, xml_)
    {
        ptree tree;
        xml_parser::read_xml(is, tree);

        read(tree, value);
    }

    static inline void write_to(std::ostream& os, T const& value, xml_)
    {
        ptree tree;
        write(tree, value);

        xml_parser::write_xml(os, tree);
    }

    static inline void read_from(std::istream& is, T& value, json_)
    {
        ptree tree;
        json_parser::read_json(is, tree);

        read(tree, value);
    }

    static inline void write_to(std::ostream& os, T const& value, json_)
    {
        ptree tree;
        write(tree, value);

        json_parser::write_json(os, tree);
    }
};

template<parser_t p = INFO,class T>
void read_from(std::istream& is, T& value)
{
    io_parser<T,p>::read_from(is, value);
}

template<parser_t p = INFO,class T>
void write_to(std::ostream& os, T const& value)
{
    io_parser<T,p>::write_to(os, value);
}

//
namespace details
{

// processors
struct write_processor
{
    explicit write_processor(ptree& pt)
        : pt(pt)
    {
    }

    template<class type>
    void operator()(char const* key, type const& object) // just forwarding to outer functions for reflection
    {
       write(pt, key, object);
    }

    ptree& pt;
};


struct read_processor
{
    explicit read_processor(ptree const& pt)
        : pt(pt)
    {
    }

    template<class type>
    void operator()(char const* key, type& object) // just forwarding to outer functions for reflection
    {
        read(pt, key, object);
    }

    ptree const& pt;
};
}

} // ptree_ser

#endif // PTREE_REFLECTION_H
