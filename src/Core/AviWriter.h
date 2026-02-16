#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <stb_image_write.h>

class AviWriter {
public:
    AviWriter(const std::string& filename, int width, int height, int fps)
        : m_width(width), m_height(height), m_fps(fps), m_frames(0), m_moviStart(0) {
        m_file.open(filename, std::ios::binary);
        if (m_file.is_open()) {
            // Write placeholder header
            // We'll overwrite this in Finish()
            // Header size is fixed:
            // RIFF (12) + LIST hdrl (200) + LIST movi (12) = 224 bytes start of movi data
            // Let's use 2048 for safety/alignment
            std::vector<char> header(2048, 0);
            m_file.write(header.data(), 2048);
            
            // We are now at the start of movi data (after "LIST size movi")
            // But we haven't written "LIST size movi" yet?
            // Actually, let's just write the placeholders for RIFF and hdrl, 
            // and start the movi list.
            
            m_moviStart = 2048; // This will be the position of "LIST" for movi
            m_file.seekp(m_moviStart);
            m_file.write("LIST", 4);
            uint32_t zero = 0;
            m_file.write((char*)&zero, 4); // Placeholder for movi size
            m_file.write("movi", 4);
        }
    }

    ~AviWriter() {
        if (m_file.is_open()) {
            Finish();
            m_file.close();
        }
    }

    bool IsOpen() const { return m_file.is_open(); }

    void WriteFrame(const unsigned char* pixels, int stride) {
        if (!m_file.is_open()) return;

        // Write chunk header "00dc"
        m_file.write("00dc", 4);
        
        long posDataStart = (long)m_file.tellp() + 4; // Data starts after size (4 bytes)
        uint32_t sizePlaceholder = 0;
        m_file.write((char*)&sizePlaceholder, 4); // Placeholder for chunk size

        // Write JPEG data using stb
        stbi_write_jpg_to_func(WriteFunc, this, m_width, m_height, 4, pixels, 90);

        long posDataEnd = m_file.tellp();
        uint32_t size = (uint32_t)(posDataEnd - posDataStart);
        
        // Go back and write size
        m_file.seekp(posDataStart - 4);
        m_file.write((char*)&size, 4);
        m_file.seekp(posDataEnd);

        // Pad to even size
        if (size % 2 != 0) {
            m_file.write("\0", 1);
            posDataEnd++;
        }

        // Record index entry
        IndexEntry entry;
        entry.id = 0x63643030; // "00dc"
        entry.flags = 0x10; // Keyframe
        // Offset is relative to the 'movi' list identifier (the "movi" tag).
        // m_moviStart is "LIST". m_moviStart+8 is "movi".
        // Chunk tag "00dc" starts at posDataStart - 8.
        entry.offset = (uint32_t)((posDataStart - 8) - (m_moviStart + 8));
        entry.size = size;
        m_index.push_back(entry);

        m_frames++;
    }

private:
    std::ofstream m_file;
    int m_width, m_height, m_fps;
    int m_frames;
    long m_moviStart;
    
    struct IndexEntry {
        uint32_t id;
        uint32_t flags;
        uint32_t offset;
        uint32_t size;
    };
    std::vector<IndexEntry> m_index;

    static void WriteFunc(void* context, void* data, int size) {
        AviWriter* writer = (AviWriter*)context;
        writer->m_file.write((char*)data, size);
    }

    void Put32(uint32_t v) {
        m_file.write((char*)&v, 4);
    }
    
    void Put16(uint16_t v) {
        m_file.write((char*)&v, 2);
    }
    
    void PutStr(const char* s) {
        m_file.write(s, 4);
    }

    void Finish() {
        long moviEnd = m_file.tellp();
        uint32_t moviSize = (uint32_t)(moviEnd - (m_moviStart + 8));
        
        // Write Index
        m_file.write("idx1", 4);
        uint32_t indexSize = (uint32_t)(m_index.size() * 16);
        Put32(indexSize);
        
        for (const auto& entry : m_index) {
            Put32(entry.id);
            Put32(entry.flags);
            Put32(entry.offset);
            Put32(entry.size);
        }
        
        long fileEnd = m_file.tellp();
        uint32_t fileSize = (uint32_t)(fileEnd - 8);
        
        // Update movi size
        m_file.seekp(m_moviStart + 4);
        Put32(moviSize);
        
        // Write Header at 0
        m_file.seekp(0);
        PutStr("RIFF");
        Put32(fileSize);
        PutStr("AVI ");
        
        // hdrl LIST
        PutStr("LIST");
        Put32(192); // Size of hdrl list data (see calc below)
        PutStr("hdrl");
        
        // avih chunk
        PutStr("avih");
        Put32(56); // Size
        uint32_t microSecPerFrame = (uint32_t)(1000000.0 / m_fps);
        Put32(microSecPerFrame); 
        Put32(m_width * m_height * 4 * m_fps); // MaxBytesPerSec (approx)
        Put32(0); // PaddingGranularity
        Put32(0x10); // Flags (AVIF_HASINDEX)
        Put32(m_frames); // TotalFrames
        Put32(0); // InitialFrames
        Put32(1); // Streams
        Put32(0); // SuggestedBufferSize
        Put32(m_width); // Width
        Put32(m_height); // Height
        Put32(0); // Reserved
        Put32(0); // Reserved
        Put32(0); // Reserved
        Put32(0); // Reserved
        
        // strl LIST
        PutStr("LIST");
        Put32(116); // Size of strl list data
        PutStr("strl");
        
        // strh chunk
        PutStr("strh");
        Put32(56); // Size
        PutStr("vids"); // fccType
        PutStr("MJPG"); // fccHandler
        Put32(0); // Flags
        Put16(0); // Priority
        Put16(0); // Language
        Put32(0); // InitialFrames
        Put32(1); // Scale
        Put32(m_fps); // Rate
        Put32(0); // Start
        Put32(m_frames); // Length
        Put32(m_width * m_height * 4); // SuggestedBufferSize
        Put32(10000); // Quality
        Put32(0); // SampleSize
        Put16(0); // Frame
        Put16(0); // Frame
        
        // strf chunk
        PutStr("strf");
        Put32(40); // Size (sizeof(BITMAPINFOHEADER))
        Put32(40); // biSize
        Put32(m_width); // biWidth
        Put32(m_height); // biHeight
        Put16(1); // biPlanes
        Put16(24); // biBitCount
        PutStr("MJPG"); // biCompression
        Put32(m_width * m_height * 3); // biSizeImage
        Put32(0); // biXPelsPerMeter
        Put32(0); // biYPelsPerMeter
        Put32(0); // biClrUsed
        Put32(0); // biClrImportant
    }
};
