#include "XexLoader.h"

#include <stdio.h>
#include <cstdint>
#include <Windows.h> // Include Windows.h for S_OK
#include <cassert>

XexImage::XexImage(const wchar_t* path) : m_path(path) // Initialize path
{
}

bool XexImage::LoadXex()
{
    // open the file
    FILE* f = nullptr;
    _wfopen_s(&f, m_path.c_str(), L"rb");
    
    if (nullptr == f)
    {
        printf("Unable to open file '%ls'", m_path.c_str());
        return false; // Return false instead of S_OK
    }

    // get file size
    fseek(f, 0, SEEK_END);
    const uint32_t fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // load data
    uint8_t* fileData = new uint8_t[fileSize];
    {
        const uint32_t bufferSize = 4096;
        for (uint32_t i = 0; i < fileSize; i += bufferSize)
        {
            const int bufferReadSize = ((fileSize - i) > bufferSize) ? bufferSize : fileSize - i;
            fread(fileData + i, bufferReadSize, 1, f);
        }
        fclose(f);
    }

    printf("Parsing headers...\n");
    ImageByteReaderXEX reader(fileData, fileSize);
    if (!LoadHeaders(reader))
    {
        delete[] fileData;
        return false;
    }

	// load, decompress and decrypt image data
	printf("Decompressing image...\n");
	if (!LoadImageData(reader))
	{
		delete[] fileData;
		return false;
	}

    delete[] fileData; // Clean up allocated memory
    return true; // Return true if successful
}


bool XexImage::LoadImageData(ImageByteReaderXEX& data)
{
	// decompress and decrypt
	const XEXCompressionType compType = m_xexData.file_format_info.compression_type;
	switch (compType)
	{
	case XEX_COMPRESSION_NONE:
	{
		printf("XEX: image::Binary is not compressed\n");
		printf("-------- COMPRESSION UNSUPPORTED ---------\n");
		//return LoadImageDataUncompressed(log, data);
	}

	case XEX_COMPRESSION_BASIC:
	{
		printf("XEX: image::Binary is using basic compression (zero blocks)\n");
		return LoadImageDataBasic(data);
	}

	case XEX_COMPRESSION_NORMAL:
	{
		printf("XEX: image::Binary is using normal compression");
		printf("-------- COMPRESSION UNSUPPORTED ---------\n");
		//return LoadImageDataNormal(log, data);
	}
	}

	// unsupported compression
	//printf("Image '%ls' is using unsupported compression mode %d and cannot be loaded\n", GetPath().c_str(), compType);
	return false;
}

bool XexImage::LoadImageDataBasic(ImageByteReaderXEX& data)
{
	// calculate the uncompressed size
	uint32_t memorySize = 0;
	const uint32_t blockCount = (uint32_t)m_xexData.file_format_info.basic_blocks.size();
	for (uint32_t i = 0; i < blockCount; ++i)
	{
		const XEXFileBasicCompressionBlock& block = m_xexData.file_format_info.basic_blocks[i];
		memorySize += block.data_size + block.zero_size;
	}

	// source data
	const uint32_t sourceSize = (uint32_t)(data.GetSize() - m_xexData.header.exe_offset);
	const uint8_t* sourceBuffer = data.GetData() + m_xexData.header.exe_offset;

	// sanity check
	const uint32_t maxImageSize = 128 << 20;
	if (memorySize >= maxImageSize)
	{
		printf("Computed image size is to big (%X), the exe offset = 0x%X\n",
			memorySize, m_xexData.header.exe_offset);

		return false;
	}

	// Allocate in-place the XEX memory.
	uint8_t* memory = (uint8_t*)malloc(memorySize);
	if (nullptr == memory)
	{
		printf("Failed to allocate image memory (size = 0x%X)\n", memorySize);
		return false;
	}

	// The decryption state is global for all blocks
	uint32_t rk[4 * (MAXNR + 1)];
	uint8_t ivec[16] = { 0 };
	int Nr = rijndaelKeySetupDec(rk, m_xexData.session_key, 128);

	// Destination memory pointers
	uint8_t* destMemory = memory;
	memset(memory, 0, memorySize);

	// Copy/Decrypt blocks
	for (uint32_t n = 0; n < blockCount; n++)
	{
		// get the size of actual data and the zeros
		const XEXFileBasicCompressionBlock& block = m_xexData.file_format_info.basic_blocks[n];
		const uint32_t data_size = block.data_size;
		const uint32_t zero_size = block.zero_size;

		// decompress/copy data
		const XEXEncryptionType encType = m_xexData.file_format_info.encryption_type;
		switch (encType)
		{
			// no encryption, copy data
		case XEX_ENCRYPTION_NONE:
		{
			memcpy(destMemory, sourceBuffer, data_size);
			break;
		}

		// AES
		case XEX_ENCRYPTION_NORMAL:
		{
			const uint8_t* ct = sourceBuffer;
			uint8_t* pt = destMemory;
			for (uint32_t n = 0; n < data_size; n += 16, ct += 16, pt += 16)
			{
				// Decrypt 16 uint8_ts from input -> output.
				rijndaelDecrypt(rk, Nr, ct, pt);

				// XOR with previous
				for (uint32_t i = 0; i < 16; i++)
				{
					pt[i] ^= ivec[i];
					ivec[i] = ct[i];
				}
			}

			break;
		}
		}

		// go to next block
		sourceBuffer += data_size;
		destMemory += data_size + zero_size;
	}

	// check if all source data was consumed
	const uint32_t consumed = (uint32_t)(sourceBuffer - data.GetData());
	if (consumed > data.GetSize())
	{
		printf("XEX: To much source data was consumed by block decompression (%d > %d)\n", consumed, data.GetSize());
		free(memory);
		return false;
	}
	else if (consumed < data.GetSize())
	{
		printf("XEX: %d bytes of data was not consumed in block decompression (out of %d)\n", data.GetSize() - consumed, data.GetSize());
	}

	// check if all data was outputed
	const uint32_t numOutputed = (uint32_t)(destMemory - memory);
	if (numOutputed > memorySize)
	{
		printf("XEX: To much data was outputed in block decompression (%d > %d)\n", numOutputed, memorySize);
		free(memory);
		return false;
	}
	else if (numOutputed < memorySize)
	{
		printf("XEX: %d bytes of data was not outputed in block decompression (out of %d)\n", memorySize - numOutputed, memorySize);
	}

	// loaded
	m_memoryData = memory;
	m_memorySize = memorySize;
	printf("XEX: Decompressed %d bytes from %d disk bytes\n", memorySize, sourceSize);
	return true;
}

bool XexImage::LoadHeaders(ImageByteReaderXEX& reader)
{
	// load the XEX header	 
	XEXImageData& imageData = m_xexData;
	XEXHeader& header = m_xexData.header;
	if (!reader.Read(&header, sizeof(header))) return false;

	// swap and validate
	header.Swap();
	if (!header.Validate()) return false;

	// parse for optional headers
	// ID & FF
	for (uint32_t n = 0; n < header.header_count; n++)
	{
		// load the header
		XEXOptionalHeader optionalHeader;
		if (!optionalHeader.Read(reader)) return false;

		// extract the length
		bool add = true;
		switch (optionalHeader.key & 0xFF)
		{
			// just the data
		case 0x00:
		case 0x01:
		{
			optionalHeader.value = optionalHeader.offset;
			optionalHeader.offset = 0;
			break;
		}

		// data
		case 0xFF:
		{
			optionalHeader.length = *(uint32_t*)(reader.GetData() + optionalHeader.offset);
			optionalHeader.offset += 4;
			Swap32(&optionalHeader.length);

			// to big ?
			if (optionalHeader.length + optionalHeader.offset >= reader.GetSize())
			{
				printf("Optional header %i (0x%X) crosses file boundary. Will not be read.\n", n, optionalHeader.key);
				add = false;
			}

			break;
		}

		// small data
		default:
		{
			optionalHeader.length = (optionalHeader.key & 0xFF) * 4;

			// to big ?
			if (optionalHeader.length + optionalHeader.offset >= reader.GetSize())
			{
				printf("Optional header %i (0x%X) crosses file boundary. Will not be read.\n", n, optionalHeader.key);
				add = false;
			}

			break;
		}
		}

		// store local optional header
		if (add)
		{
			m_xexData.optional_headers.push_back(optionalHeader);
		}
	}

	// process the optional headers
	for (uint32_t i = 0; i < m_xexData.optional_headers.size(); ++i)
	{
		const XEXOptionalHeader& opt = m_xexData.optional_headers[i];

		// go to the header offset
		if (opt.length > 0 && opt.offset != 0)
		{
			if (!reader.Seek(opt.offset))
			{
				continue;
			}
		}

		// process the optional headers
		switch (opt.key)
		{
			// System flags
		case XEX_HEADER_SYSTEM_FLAGS:
		{
			imageData.system_flags = (XEXSystemFlags)opt.value;
			break;
		}

		// Resource info repository
		case XEX_HEADER_RESOURCE_INFO:
		{
			// get the count
			const uint32_t count = (opt.length - 4) / 16;
			m_xexData.resources.resize(count);

			// move to file position
			for (uint32_t n = 0; n < count; ++n)
			{
				// load the resource entry
				XEXResourceInfo& resInfo = m_xexData.resources[i];
				if (!reader.Read(&resInfo, sizeof(resInfo))) return false;
			}

			break;
		}

		// Execution info
		case XEX_HEADER_EXECUTION_INFO:
		{
			if (!imageData.execution_info.Read(reader)) return false;
			break;
		}

		// TLS info
		case XEX_HEADER_TLS_INFO:
		{
			if (!imageData.tls_info.Read(reader)) return false;
			break;
		}

		// Base address
		case XEX_HEADER_IMAGE_BASE_ADDRESS:
		{
			imageData.exe_address = opt.value;
			printf("XEX: Found base addrses: 0x%08X\n", imageData.exe_address);
			break;
		}

		// Entry point
		case XEX_HEADER_ENTRY_POINT:
		{
			imageData.exe_entry_point = opt.value;
			printf("XEX: Found entry point: 0x%08X\n", imageData.exe_entry_point);
			break;
		}

		// Default stack size
		case XEX_HEADER_DEFAULT_STACK_SIZE:
		{
			imageData.exe_stack_size = opt.value;
			break;
		}

		// Default heap size
		case XEX_HEADER_DEFAULT_HEAP_SIZE:
		{
			imageData.exe_heap_size = opt.value;
			break;
		}

		// File format information
		case XEX_HEADER_FILE_FORMAT_INFO:
		{
			// load the encryption type
			XEXEncryptionHeader encHeader;
			if (!encHeader.Read( reader)) return false;

			// setup header info
			imageData.file_format_info.encryption_type = (XEXEncryptionType)encHeader.encryption_type;
			imageData.file_format_info.compression_type = (XEXCompressionType)encHeader.compression_type;

			// load compression blocks
			switch (encHeader.compression_type)
			{
			case XEX_COMPRESSION_NONE:
			{
				printf("XEX: image::Binary is using no compression\n");
				break;
			}

			case XEX_COMPRESSION_DELTA:
			{
				printf("XEX: image::Binary is using unsupported delta compression\n");
				break;
			}

			case XEX_COMPRESSION_BASIC:
			{
				// get the block count
				const uint32_t block_count = (opt.length - 8) / 8;
				imageData.file_format_info.basic_blocks.resize(block_count);

				// load the basic compression blocks
				for (uint32_t i = 0; i < block_count; ++i)
				{
					XEXFileBasicCompressionBlock& block = imageData.file_format_info.basic_blocks[i];

					if (!reader.Read( &block, sizeof(block))) return false;
					Swap32(&block.data_size);
					Swap32(&block.zero_size);
				}

				printf("XEX: image::Binary is using basic compression with %d blocks\n", block_count);
				break;
			}

			case XEX_COMPRESSION_NORMAL:
			{
				XEXFileNormalCompressionInfo& normal_info = imageData.file_format_info.normal;
				if (!normal_info.Read( reader)) return false;

				printf("XEX: image::Binary is using normal compression with block size = %d\n", normal_info.block_size);
				break;
			}
			}

			// encryption type
			if (encHeader.encryption_type != XEX_ENCRYPTION_NONE)
			{
				printf("XEX: image::Binary is encrypted\n");
			}

			// opt header
			break;
		}

		// Import libraries - very important piece
		case XEX_HEADER_IMPORT_LIBRARIES:
		{
			// Load the header data
			XEXImportLibraryBlockHeader blockHeader;
			if (!blockHeader.Read( reader)) return false;

			// get the string data
			const char* string_table = (const char*)reader.GetData() + reader.GetOffset();
			reader.Seek( reader.GetOffset() + blockHeader.string_table_size);

			// load the imports
			for (uint32_t m = 0; m < blockHeader.count; m++)
			{
				XEXImportLibraryHeader header;
				if (!header.Read( reader)) return false;

				// get the library name
				const char* name = "Unknown";
				const uint16_t name_index = header.name_index & 0xFF;
				for (uint32_t i = 0, j = 0; i < blockHeader.string_table_size; )
				{
					assert(j <= 0xFF);
					if (j == name_index)
					{
						name = string_table + i;
						break;
					}
					
					if (string_table[i] == 0)
					{
						i++;
						if (i % 4)
						{
							i += 4 - (i % 4);
						}
						j++;
					}
					else
					{
						i++;
					}
				}

				// save the import lib name
				if (name[0])
				{
					printf("Found import library: '%s'\n", name);
					m_libNames.push_back(name);
				}

				// load the records
				for (uint32_t i = 0; i < header.record_count; ++i)
				{
					// load the record entry
					uint32_t recordEntry = 0;
					if (!reader.Read( &recordEntry, sizeof(recordEntry))) return false;
					Swap32(&recordEntry);

					// add to the global record entry list
					m_xexData.import_records.push_back(recordEntry);
				}
			}

			// done
			break;
		}
		}
	}

	// load the loader info
	{
		// go to the certificate region
		if (!reader.Seek( header.certificate_offset)) return false;

		// load the loader info
		XEXLoaderInfo& li = m_xexData.loader_info;
		if (!li.Read( reader)) return false;

		// print some stats
		printf("XEX: Binary size: 0x%X\n", li.image_size);
	}

	// load the sections
	{
		// go to the section region
		if (!reader.Seek( header.certificate_offset + 0x180)) return false;

		// get the section count
		uint32_t sectionCount = 0;
		if (!reader.Read( &sectionCount, sizeof(sectionCount))) return false;
		Swap32(&sectionCount);

		// load the sections
		m_xexData.sections.resize(sectionCount);
		for (uint32_t i = 0; i < sectionCount; ++i)
		{
			XEXSection& section = m_xexData.sections[i];
			if (!reader.Read( &section, sizeof(XEXSection))) return false;
			Swap32(&section.info.value);
		}
	}

	/*/ decrypt the XEX key
	{
		// Key for retail executables
		const static uint8 xe_xex2_retail_key[16] =
		{
			0x20, 0xB1, 0x85, 0xA5, 0x9D, 0x28, 0xFD, 0xC3,
			0x40, 0x58, 0x3F, 0xBB, 0x08, 0x96, 0xBF, 0x91
		};

		// Key for devkit executables
		const static uint8 xe_xex2_devkit_key[16] =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		// Guess key based on file info.
		const uint8* keyToUse = xe_xex2_devkit_key;
		if (m_xexData.execution_info.title_id != 0)
		{
			printf("XEX: Found TitleID 0x%X", m_xexData.execution_info.title_id);
			//if ( m_xexData.system_flags 
			keyToUse = xe_xex2_retail_key;
		}

		// Decrypt the header and session key
		uint32_t buffer[4 * (MAXNR + 1)];
		int32 nr = rijndaelKeySetupDec(buffer, keyToUse, 128);
		rijndaelDecrypt(buffer, nr, m_xexData.loader_info.file_key, m_xexData.session_key);

		// stats
		{
			const uint32_t* keys = (const uint32_t*)&m_xexData.loader_info.file_key;
			printf("XEX: Decrypted file key: %08X-%08X-%08X-%08X", keys[0], keys[1], keys[2], keys[3]);

			const uint32_t* skeys = (const uint32_t*)&m_xexData.session_key;
			printf("XEX: Decrypted session key: %08X-%08X-%08X-%08X", skeys[0], skeys[1], skeys[2], skeys[3]);
		}
	}*/

	// headers loaded
	return true;
}