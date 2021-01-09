#ifndef VXIO_MINIZ_CPP_HPP
#define VXIO_MINIZ_CPP_HPP

#include "miniz_cppfwd.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <functional>

namespace miniz_cpp {

struct zip_info
{
    std::string filename;

    struct date_time {
        int year = 1980;
        int month = 0;
        int day = 0;
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
    } date_time;

    std::string comment;
    std::string extra;
    uint16_t create_system = 0;
    uint16_t create_version = 0;
    uint16_t extract_version = 0;
    uint16_t flag_bits = 0;
    std::size_t volume = 0;
    uint32_t internal_attr = 0;
    uint32_t external_attr = 0;
    std::size_t header_offset = 0;
    uint32_t crc = 0;
    std::size_t compress_size = 0;
    std::size_t file_size = 0;
};

class zip_file
{
public:
    zip_file();
    ~zip_file();

    zip_file(const std::string &filename) : zip_file()
    {
        load(filename);
    }

    zip_file(std::istream &stream) : zip_file()
    {
        load(stream);
    }

    zip_file(const std::vector<unsigned char> &bytes) : zip_file()
    {
        load(bytes);
    }

    void load(std::istream &stream);
    void load(const std::string &filename);
    void load(const std::vector<unsigned char> &bytes);

    void save(const std::string &filename);
    void save(const std::function<void(const char* data, size_t size)> &consumer);
    void save(std::ostream &stream);
    void save(std::vector<unsigned char> &bytes);

    void reset();

    bool has_file(const std::string &name);
    bool has_file(const zip_info &name);

    zip_info getinfo(const std::string &name);

    std::vector<zip_info> infolist();

    std::vector<std::string> namelist();

    std::ostream &open(const std::string &name);
    std::ostream &open(const zip_info &name);

    void extract(const std::string &member, const std::string &path);
    void extract(const zip_info &member, const std::string &path);

    void extractall(const std::string &path);
    void extractall(const std::string &path, const std::vector<std::string> &members);
    void extractall(const std::string &path, const std::vector<zip_info> &members);

    void printdir();
    void printdir(std::ostream &stream);

    std::string read(const zip_info &info);
    std::string read(const std::string &name);

    std::pair<bool, std::string> testzip();

    void write(const std::string &filename);
    void write(const std::string &filename, const std::string &arcname);

    void writestr(const std::string &arcname, const std::string &bytes);
    void writestr(const zip_info &info, const std::string &bytes);

    std::string get_filename() const { return filename_; }

    std::string comment;

private:
    void start_read();
    void start_write();

    void append_comment();

    void remove_comment();

    zip_info getinfo(int index);

    std::unique_ptr<mz_zip_archive_tag> archive_;
    std::vector<char> buffer_;
    std::stringstream open_stream_;
    std::string filename_;
};

} // namespace miniz_cpp

#endif // VXIO_MINIZ_CPP_HPP
