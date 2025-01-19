#pragma once
#include "rijndael-alg-fst.h"
#include <string>
#include "XEXImage.h"


class XexImage
{
public:
	XexImage(const wchar_t* path);

	//! Load from file, returns the loaded image
	bool LoadXex();
	bool LoadHeaders(ImageByteReaderXEX& reader);
	bool LoadImageData(ImageByteReaderXEX& data);

	// Compression
	bool LoadImageDataBasic(ImageByteReaderXEX& data);

private:
	// file header
	XEXImageData			m_xexData;
	PEOptHeader				m_peHeader;

	// library names
	std::vector< std::string >		m_libNames;

	// version info for the import module
	XEXVersion				m_version;
	XEXVersion				m_minimalVersion;


	uint32_t					m_memorySize;
	const uint8_t* m_memoryData;
	std::wstring m_path;
};



