#ifndef FILE_HPP
#define FILE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

namespace io {
    class File {
        public:
            File(const string& path);
            ~File();
            
            [[nodiscard]] bool operator!() const;

            void clear();

            template <typename T>
            [[nodiscard]] T read(size_t offset = 0) {
                if constexpr (is_same_v<T, string>) {
                    string data;
                    m_stream.seekg(offset);

                    string line;
                    while (getline(m_stream, line)) {
                        data += line + "\n";
                    }
                    data.pop_back();

                    return data;
                } 
                
                T data;
                m_stream.seekg(offset);
                m_stream.read(reinterpret_cast<char*>(&data), sizeof(T));
                return data;
            }

            template <typename T>
            void write(const T& data, size_t offset = 0) {
                if constexpr (is_same_v<T, string>) {
                    m_stream.seekp(offset);
                    m_stream << data;
                    return;
                }
             
                m_stream.seekp(offset);
                m_stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
            }

        private:
            string m_filename;
            fstream m_stream;
    };
}

#endif