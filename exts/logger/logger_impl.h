#ifndef LOGGER_IMPL_H
#define LOGGER_IMPL_H

#include <iostream>
#include <fstream>

namespace tlog
{
    using namespace std;

    inline properties::properties(level default_level)
        : def_level(default_level)
    {
    }

    inline properties::properties(string filename, level default_level)
        : def_level(default_level)
    {
        if (filename.empty())
            return;

        std::ifstream in(filename.c_str());

        map<string, level> remap;

        remap.insert(make_pair("TRACE", L_TRACE));
        remap.insert(make_pair("DEBUG", L_DEBUG));
        remap.insert(make_pair("INFO" , L_INFO) );
        remap.insert(make_pair("WARN" , L_WARN) );
        remap.insert(make_pair("ERROR", L_ERROR));


        while (!in.eof())
        {
            //using namespace boost;

            char buf [0x400] = {};
            in.getline(buf, sizeof buf);

            string str(buf);
            //trim(str);

            //if (str.empty() || boost::starts_with(str, "#"))
            //	continue;

            //vector<string> entry;
            //split(entry, str, is_any_of("="));

            //if(entry.size() != 2)
            //	throw std::runtime_error("invalid log file property entry: " + str);

            //trim    (entry[0]);
            //trim    (entry[1]);
            //to_upper(entry[1]);

            //auto it = remap.find(entry[1]);
            //if(it == remap.end())
            //	throw std::runtime_error("invalid log file level description: " + entry[1]);

            //levels[entry[0]] = it->second;
        }

        levels_t::iterator def = levels.find("default");
        if (def != levels.end())
            def_level = def->second;
    }


//////////////////////////////////////////////////////////
    inline logger::logger(properties const& props)
        : props_(props)
    {
    }

    inline void logger::add_writer(writer_ptr ptr)
    {
        writers_.push_back(ptr);
    }


    inline bool logger::need_log(level desired_level, const char* source) const
    {
        level source_level = props_.def_level;

        properties::levels_t::const_iterator it = props_.levels.find(source);
        if (it != props_.levels.end())
            source_level = it->second;

        return desired_level >= source_level && !writers_.empty();
    }

    inline void logger::log(level desired_level, const char* file, size_t line_num, const char* source, const char* msg)
    {
        double time = timer_.time();

        for(writers_t::iterator it = writers_.begin(); it != writers_.end(); ++it)
            (*it)->write_msg(time, desired_level, file, line_num, source, msg);
    }

}
#endif // LOGGER_IMPL_H
