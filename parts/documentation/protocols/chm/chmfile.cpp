/*  This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qfile.h>
#include "chmfile.h"
#include "decompress.h"

uint Chm::getEncInt(QFile& f, uint &value) const
{
    int c;
    uint result = 0;
    ulong count = 0;

    do
    {
        c = f.getch();
        result <<= 7;
        result |= (c & 0x7F);
        count++;
    } while (c & 0x80);

    value = result;
    return count;
}

uint Chm::getName(QFile& f, QString& name) const
{
    int len = f.getch();
    char *buf = new char[len];
    f.readBlock(buf, len);
    name = QString::fromUtf8(buf, len);
    if (name.startsWith("/"))
        name = name.lower();
    delete buf;
    return len + 1;
}

uint Chm::getIntel32(QFile& f) const
{
    uint value = f.getch() | f.getch() << 8 | f.getch() << 16 | f.getch() << 24;
    return value;
}

uint Chm::getIntel64(QFile& f) const
{
    uint value = getIntel32(f);
    f.at(f.at() + 4);
    return value;
}

bool Chm::getChunk(QFile& f, uint chunkSize, ChmDirectoryMap& directoryMap) const
{
    char tag[4];
    if (f.readBlock(tag, 4) != 4) return false;

    if (!qstrncmp(tag, "PMGL", 4))
    {
        uint quickref_length = getIntel32(f);
        f.at(f.at() + 12);

        uint pos = 20;
        while (pos < chunkSize - quickref_length)
        {
            uint section, offset, length;
            QString name;
            pos += getName(f, name);
            pos += getEncInt(f, section);
            pos += getEncInt(f, offset);
            pos += getEncInt(f, length);
            directoryMap[name] = ChmDirTableEntry(section, offset, length);
            if (name.endsWith(".hhc"))
                directoryMap["/@contents"] = ChmDirTableEntry(section, offset, length);
        }

        return (f.at(f.at() + quickref_length));
    }
    else if (!qstrncmp(tag, "PMGI", 4))
    {
        // evaluation of the index chunk is not yet implemented => skip it
        return f.at(f.at() + chunkSize - 4);
    }
    else
    {
        return false;
    }
}

bool Chm::read(const QString& fileSpec, ChmDirectoryMap& dirMap, QByteArray& contents) const
{
    QFile f(fileSpec);
    if (!f.open(QIODevice::ReadOnly)) return false;

    // read CHM file header
    char tag[4];
    if (f.readBlock(tag, 4) != 4 || qstrncmp(tag, "ITSF", 4)) return false;
	uint chm_version = getIntel32(f);
    if (!f.at(f.at() + 0x30)) return false;

    // read header section table
    uint section_0_offset = getIntel64(f);
    uint section_0_length = getIntel64(f);
    uint section_1_offset = getIntel64(f);
    uint section_1_length = getIntel64(f);

	uint contentStart = 0;
	if (chm_version >= 3) contentStart = getIntel32(f);

    // read directory header
    if (!f.at(section_1_offset)) return false;
    if (f.readBlock(tag, 4) != 4 || qstrncmp(tag, "ITSP", 4)) return false;
    if (!f.at(f.at() + 12)) return false;
    uint directory_chunk_size = getIntel32(f);
    if (!f.at(f.at() + 24)) return false;
    uint num_directory_chunks = getIntel32(f);
    if (!f.at(f.at() + 36)) return false;

    // read directory table
    for (uint i = 0; i < num_directory_chunks; i++)
        if (!getChunk(f, directory_chunk_size, dirMap)) return false;

    // current position is start of content area
	if (chm_version < 3) contentStart = f.at();

    // read reset table
    if (!f.at(contentStart)) return false;
    uint resetTableOffset =
    dirMap["::DataSpace/Storage/MSCompressed/Transform/{7FC28940-9D31-11D0-9B27-00A0C91E9C7C}/InstanceData/ResetTable"].offset;
    if (!f.at(f.at() + resetTableOffset + 4)) return false;
    uint numResetTableEntries = getIntel32(f);
    if (!f.at(f.at() + 8)) return false;
    uint uncompressedLength = getIntel64(f);
    uint compressedLength = getIntel64(f);
	uint blockSize = getIntel64(f);
    uint *resetTable = new uint[numResetTableEntries + 1];

    for (uint i = 0; i < numResetTableEntries; i++)
        resetTable[i] = getIntel64(f);

    resetTable[numResetTableEntries] = compressedLength;

    // read compressed contents
    if (!f.at(contentStart)) return false;
    uint contentsOffset = dirMap["::DataSpace/Storage/MSCompressed/Content"].offset;
    if (!f.at(f.at() + contentsOffset)) return false;
    char *compressedContents = new char[compressedLength];
    if ((uint)f.readBlock(compressedContents, compressedLength) != compressedLength) return false;

	f.close();

    // allocate buffer for uncompressed contents
    char *uncompressedContents = new char[uncompressedLength];

    // get window size
	uint window = 1;
	uint tmp = blockSize;
	while (tmp >>= 1) window++;
	
	// decompress
    uint outlen = uncompressedLength;
    int res = 1;

    for (uint i = 0; i < numResetTableEntries; i++)
    {
        if (!(i & 1)) LZXinit(window);

        uint inlen = resetTable[i+1] - resetTable[i];
        res = LZXdecompress((uchar*)&compressedContents[resetTable[i]],
                            inlen,
                            (uchar*)uncompressedContents + i * blockSize,
                            (outlen < blockSize) ? outlen : blockSize);
        if (res) break;
        outlen -= blockSize;
    }

    delete [] resetTable;
    delete [] compressedContents;

    if (res == 0)
        contents.duplicate(uncompressedContents, uncompressedLength);

	delete [] uncompressedContents;

    return (res == 0);
}
