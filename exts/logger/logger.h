#ifndef LOGGER_H
#define LOGGER_H

namespace tlog
{
    using namespace std;

    enum level
    {
        L_ERROR,
        L_WARN,
        L_INFO,
        L_DEBUG,
        L_TRACE
    };

    inline string level_name(level l)
    {
        static string names [] =
        {
            "ERROR",
            "WARN" ,
            "INFO" ,
            "DEBUG",
            "TRACE"
        };

        return names[l];
    }

    class writer
    {
    public:
        virtual void write_msg( level l,  const char* msg) = 0;
        virtual ~writer(){}
    };

    typedef
        shared_ptr<writer> writer_ptr;
/////////////////////////////////////////////////////
    class perf_counter
    {
    public:
        double time(){return 0;};
    };
/////////////////////////////////////////////////////
    struct properties
    {
        /* Reading logger properties from file
         *
         * File should contains lines like below.
         * Use leading # for comment.
         * File could contain line with default behavior description. If no 'default' entry found, default is set to INFO
         * Valid levels: TRACE, DEBUG, INFO, WARN, ERROR
         *
         *	default=INFO
         *	AbraCaDabra=TRACE
         *	#BubaZuba=WARN
         *	HariKrishna=INFO
         *
         */

        properties(string filename = "", level default_level = L_INFO);
        properties(level default_level);

        typedef std::map<string, level>		levels_t;

        levels_t							levels;
        level								def_level;
    };

///////////////////////////////////////////////////
    class logger
    {
        typedef std::vector<writer_ptr> writers_t;

    public:
        logger(properties const& props);

        inline static logger & logger::get_logger()
        {
            static properties pr;
            static logger lg(pr);
            return lg;
        }

        bool need_log(level desired_level, const char* source) const;
        void log(level desired_level, const char* file, size_t line_num, const char* source, const char* msg);
        void add_writer(writer_ptr ptr);

    private:
        properties 				props_  ;
        writers_t				writers_;
        perf_counter            timer_  ;
    };

} // namespace tlog


inline const char* __scope_source_name()
{
    return " General ";
}

#define DECL_LOGGER(source)                                                             \
static const char* __scope_source_name()                                                \
{                                                                                       \
    return source;                                                                      \
}

#define LOG_MSG(l, msg)                                                                             \
{                                                                                                   \
    tlog::logger& __lg__ = tlog::logger::get_logger();                                              \
                                                                                                    \
    if (__lg__.need_log((l), __scope_source_name()))                                                \
    {                                                                                               \
        std::stringstream __logger__str__;                                                          \
        __logger__str__ << msg; /* don't use brackets here! */                                      \
        __lg__.log((l), __FILE__, __LINE__, __scope_source_name(), __logger__str__.str().c_str());  \
    }                                                                                               \
}

#define T_DEBUG(message)                      LOG_MSG(tlog::L_DEBUG, message)
#define T_TRACE(message)                      LOG_MSG(tlog::L_TRACE, message)
#define T_INFO(message)                       LOG_MSG(tlog::L_INFO , message)
#define T_WARN(message)                       LOG_MSG(tlog::L_WARN, message)
#define T_ERROR(message)                      LOG_MSG(tlog::L_ERROR, message)

/// implementation /////////////////////////////////////
#include "logger_impl.h"

#endif // LOGGER_H
