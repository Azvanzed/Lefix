#include "file.hpp"
#include <fstream>

using namespace std;

namespace io {
    File::File(const string& filename) : m_filename(move(filename)) {
        m_stream.open(filename, ios::in | ios::out | ios::binary | ios::app);
    }

    File::~File() {
        m_stream.close();
    }
                
    bool File::operator!() const {
        return !m_stream;
    }

    void File::clear() {
        m_stream.close();
        m_stream.open(m_filename, std::ios::out | std::ios::trunc);
        m_stream.close();
        m_stream.open(m_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    }
}