#ifndef BIN_STREAMS_H
#define BIN_STREAMS_H

namespace bin_ser
{

typedef char*               data_raw_ptr;
typedef char const*         data_raw_cptr;

typedef vector<char>        data_t;
typedef data_t&             data_ref;
typedef data_t const&       data_cref;

inline data_raw_ptr  raw_ptr(data_ref  data) { return &data.at(0); }
inline data_raw_cptr raw_ptr(data_cref data) { return &data.at(0); }

template<class T>
T& look_like(data_ref data) { return *reinterpret_cast<T*>(raw_ptr(data)); }

// streams
struct output_stream
{
    output_stream()
    {
    }

    output_stream(data_t&& stream)
        : data_(std::forward<data_t>(stream))
    {
    }

    void write(void const* data, size_t size) // size in bytes
    {
        char const* raw_ptr = (char const*)data;
        data_.insert(data_.end(), raw_ptr, raw_ptr + size);
    }

    data_t const& raw_data() const
    {
        return data_;
    }

    data_t detach()
    {
        return std::move(data_);
    }

private:
    data_t data_;
};


struct input_stream
{
    input_stream(void const* data, size_t size)
        : begin_(static_cast<const char*>(data))
        , end_  (begin_ + size)
        , cur_  (begin_)
    {
    }

    void read(void* data, size_t size) // size in bytes
    {
        assert(end_ - cur_ >= ptrdiff_t(size));

        memcpy(data, cur_, size);
        cur_ += size;
    }

    bool        eof () const { return cur_ == end_; }
    size_t      left() const { return end_ - cur_ ; }
    const char* curr() const { return cur_; }

    void        reset()             { cur_ = begin_; }
    void        skip (size_t size)  { /*Verify(end_ - cur_ >= ptrdiff_t(size));*/ cur_ += size ;}


private:
    char const* begin_;
    char const* end_  ;
    char const* cur_  ;
};


} // bin_ser

#endif // BIN_STREAMS_H
