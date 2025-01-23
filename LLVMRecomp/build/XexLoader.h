#pragma once
#include "XEXImage.h"
#include <string>

class Section;
class XexImage;
/*


void Section::Save(IBinaryFileWriter& writer) const
{
        FileChunk chunk(writer, "Section");
        writer << m_name;
        writer << m_virtualOffset;
        writer << m_virtualSize;
        writer << m_physicalOffset;
        writer << m_physicalSize;
        writer << m_isReadable;
        writer << m_isWritable;
        writer << m_isExecutable;
        writer << m_cpuName;
}

bool Section::Load(Binary* image, IBinaryFileReader& reader)
{
        FileChunk chunk(reader, "Section");
        if (!chunk)
                return false;

        reader >> m_name;
        reader >> m_virtualOffset;
        reader >> m_virtualSize;
        reader >> m_physicalOffset;
        reader >> m_physicalSize;
        reader >> m_isReadable;
        reader >> m_isWritable;
        reader >> m_isExecutable;
        reader >> m_cpuName;

        m_image = image;
        return true;
}

const bool Section::IsValidOffset(const uint32 offset) const
{
        return (offset >= m_virtualOffset) && (offset < (m_virtualOffset + m_virtualSize));
}
*/
class Section {
public:
  inline XexImage *GetImage() const {
    return m_image;
  }

  inline const std::string &GetName() const {
    return m_name;
  }
  inline const uint32_t GetVirtualOffset() const {
    return m_virtualOffset;
  }
  inline const uint32_t GetVirtualSize() const {
    return m_virtualSize;
  }
  inline const uint32_t GetPhysicalOffset() const {
    return m_physicalOffset;
  }
  inline const uint32_t GetPhysicalSize() const {
    return m_physicalSize;
  }
  inline const bool CanRead() const {
    return m_isReadable;
  }
  inline const bool CanWrite() const {
    return m_isWritable;
  }
  inline const bool CanExecute() const {
    return m_isExecutable;
  }
  inline const std::string &GetCPUName() const {
    return m_cpuName;
  }

  Section()
    : m_name("none")
    , m_cpuName("none")
    , m_virtualOffset(0)
    , m_virtualSize(0)
    , m_physicalOffset(0)
    , m_physicalSize(0)
    , m_isReadable(false)
    , m_isWritable(false)
    , m_isExecutable(false) {
  }
  Section(XexImage *parent,
    const char *name,
    const uint32_t virtualAddress,
    const uint32_t virtualSize,
    const uint32_t physicalAddress,
    const uint32_t physicalSize,
    const bool isReadable,
    const bool isWritable,
    const bool isExecutable,
    const char *cpuName);

  // save/load the object
  // void Save(IBinaryFileWriter& writer) const;
  // bool Load(XexImage* image, IBinaryFileReader& reader);

  // Check if specified offset is valid offset within the section
  const bool IsValidOffset(const uint32_t offset) const {
    return (offset >= m_virtualOffset) && (offset < (m_virtualOffset + m_virtualSize));
  }

private:
  XexImage *m_image;
  std::string m_name;

  uint32_t m_virtualOffset;
  uint32_t m_virtualSize;

  uint32_t m_physicalOffset;
  uint32_t m_physicalSize;

  bool m_isReadable;
  bool m_isWritable;
  bool m_isExecutable;

  std::string m_cpuName;
};

class XexImage {
public:
  XexImage(const wchar_t *path);

  //! Load from file, returns the loaded image
  bool LoadXex();
  bool LoadHeaders(ImageByteReaderXEX &reader);
  bool LoadImageData(ImageByteReaderXEX &data);

  // Compression
  bool LoadImageDataBasic(ImageByteReaderXEX &data);

  // PE
  bool LoadPEImage(const uint8_t *fileData, const uint32_t fileDataSize);
  Section *CreateSection(const COFFSection &section);

  inline const std::wstring &GetPath() const {
    return m_path;
  }

  inline const uint32_t GetMemorySize() const {
    return m_memorySize;
  }
  inline const uint8_t *GetMemory() const {
    return m_memoryData;
  }

  inline const uint64_t GetBaseAddress() const {
    return m_baseAddress;
  }
  inline const uint64_t GetEntryAddress() const {
    return m_entryAddress;
  }

  inline const uint32_t GetNumSections() const {
    return (uint32_t)m_sections.size();
  }
  inline const Section *GetSection(const uint32_t index) const {
    return m_sections[index];
  }

  /*inline const uint32_t GetNumImports() const { return (uint32_t)m_imports.size(); }
  inline const Import* GetImport(const uint32_t index) const { return m_imports[index]; }

  inline const uint32_t GetNumExports() const { return (uint32_t)m_exports.size(); }
  inline const Export* GetExports(const uint32_t index) const { return m_exports[index]; }

  inline const uint32_t GetNumSymbols() const { return (uint32_t)m_symbols.size(); }
  inline const Symbol* GetSymbol(const uint32_t index) const { return m_symbols[index]; }*/

  uint8_t *m_textData;

private:
  // file header
  XEXImageData m_xexData;
  PEOptHeader m_peHeader;

  // library names
  std::vector<std::string> m_libNames;

  // version info for the import module
  XEXVersion m_version;
  XEXVersion m_minimalVersion;

  uint32_t m_memorySize;
  const uint8_t *m_memoryData;
  std::wstring m_path;

  uint64_t m_baseAddress;
  uint64_t m_entryAddress;

  std::vector<Section *> m_sections;

  /*typedef std::vector< Import* >		TImports;
  TImports				m_imports;

  typedef std::vector< Export* >		TExports;
  TExports				m_exports;

  typedef std::vector< Symbol* >	TSymbols;
  TSymbols				m_symbols;*/
};
