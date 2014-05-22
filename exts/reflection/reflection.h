#ifndef REFLECTION_H
#define REFLECTION_H

// for using outside structure or class
#define REFL_STRUCT(type)                               \
template<class processor>                               \
    void reflect(processor& proc, type const& object)   \
{                                                       \
    type& obj = const_cast<type&>(object); // small hack


// for using inside structure or class - mostly for templates
#define REFL_INNER(type)                                    \
template<class processor>                                   \
    friend                                                  \
    void reflect(processor& proc, type const& object)       \
{                                                           \
    type& obj = const_cast<type&>(object);


#define REFL_ENTRY(entry)                               \
    proc(#entry, obj.entry);                            \

#define REFL_CHAIN(base)                                \
    reflect(proc, (base&)obj);

#define REFL_END()                                      \
}


//////////////////////////////////////////////////////////////////////////
// simple sample:

// struct group
// {
//     string name;
// };
//
// REFL_STRUCT(group)
//     REFL_ENTRY(name)
// REFL_END   ()
//
// struct user
// {
//     string          name;
//     vector<group>   groups;
//
//     REFL_INNER(person)
//         REFL_ENTRY(name)
//         REFL_ENTRY(groups)
//     REFL_END  ()
// };
//

#endif // REFLECTION_H
