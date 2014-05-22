#ifndef REFL_AFX_H
#define REFL_AFX_H

#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits.hpp>

#include "reflection/bin_reflection.h"
#include "reflection/bin_streams.h"

#define BOOST_REFLECTION
#include "trx_data.h"

using namespace boost;
#include "reflection/ptree_reflection.h"


#endif // REFL_AFX_H
