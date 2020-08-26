#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>

#include <Windows.h>

#include "gsm.h"

using std::string;
using std::vector;

template<typename T> T read(std::istream & in)
{
	T value;
	in.read(reinterpret_cast<char *>(&value), sizeof(T));
	return value;
}

template<typename T> void write(std::ostream & out, T value)
{
	out.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

void ensureFinalSlash(string & s)
{
	if (!s.empty())
	{
		int c = s.back();
		if (c != '/' && c != '\\')
			s += '/';
	}
}

bool convertFile(const string & inFileName, const string & outFileName)
{
	std::ifstream in(inFileName, std::ios::binary);
	if (!in.is_open())
	{
		fprintf(stderr, "Unable to open %s\n", inFileName.c_str());
		return false;
	}

	std::ofstream out(outFileName, std::ios::binary);
	if (!out.is_open())
	{
		fprintf(stderr, "Unable to open output file %s\n", outFileName.c_str());
		return false;
	}

	unsigned int hdr0 = read<unsigned>(in);
	unsigned int hdr1 = read<unsigned>(in);
	unsigned int hdr2 = read<unsigned>(in);
	unsigned int bytes_of_data = read<unsigned>(in);
	unsigned int sample_rate = read<unsigned>(in);

	bool bad_header = false;

	if (hdr0 != 0x98765432)
	{
		fprintf(stderr, "%s: Expected first dword to be 0x98765432; got %08x instead\n", inFileName.c_str(), hdr0);
		bad_header = true;
	}
	if (hdr1 != 1)
	{
		fprintf(stderr, "%s: Expected second dword to be 1; got %08x instead\n", inFileName.c_str(), hdr1);
		bad_header = true;
	}
	if (hdr2 != 0x3f800000)
	{
		fprintf(stderr, "%s: Expected third dword to be 0x3f800000; got %08x instead\n", inFileName.c_str(), hdr2);
		bad_header = true;
	}
	if (bytes_of_data % 33 != 0)
	{
		fprintf(stderr, "%s: Expected an even multiple of 33 bytes of data; got %d instead\n", inFileName.c_str(), bytes_of_data);
		bad_header = true;
	}

	if (bad_header)
		return false;

	unsigned int num_frames = bytes_of_data / 33;
	unsigned int num_samples = num_frames * 160;

	// Write a WAV header.
	write<unsigned>(out, 0x46464952); // 'RIFF'
	write<unsigned>(out, 2*num_samples + 36); // file size minus first 8 bytes
	write<unsigned>(out, 0x45564157); // 'WAVE'
	write<unsigned>(out, 0x20746d66); // 'fmt '
	write<unsigned>(out, 16); // size of fmt chunk
	write<unsigned short>(out, 1); // audio format (uncompressed PCM)
	write<unsigned short>(out, 1); // # channels
	write<unsigned>(out, sample_rate); // sample rate
	write<unsigned>(out, sample_rate*2); // byte rate
	write<unsigned short>(out, 2); // block align
	write<unsigned short>(out, 16); // bits per sample
	write<unsigned>(out, 0x61746164); // 'data'
	write<unsigned>(out, 2*num_samples); // data chunk size

	gsm codec = gsm_create();

	for (unsigned int j = 0; j < num_frames; ++j)
	{
		gsm_frame frame;
		short decoded_signal[160];
		in.read(reinterpret_cast<char *>(&frame), sizeof(frame));
		gsm_decode(codec, frame, decoded_signal);
		out.write(reinterpret_cast<const char *>(&decoded_signal), sizeof(decoded_signal));
	}

	gsm_destroy(codec);

	return true;
}

vector<string> filesInDir(const string & dir, const string & pattern)
{
    vector<string> names;
	string searchPattern = dir + pattern;
    WIN32_FIND_DATA fd; 
    HANDLE hFind = ::FindFirstFile(searchPattern.c_str(), &fd); 
    if (hFind != INVALID_HANDLE_VALUE)
	{ 
        do
		{ 
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				string fileName = dir;
				fileName.append(fd.cFileName);

                names.push_back(fileName);
            }
        }
		while(::FindNextFile(hFind, &fd)); 

        ::FindClose(hFind); 
    } 
    return names;
}

int main(int argc, const char * argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: sfx2wav <input directory> <output directory>\n");
		return -1;
	}

	string indir = argv[1];
	string outdir = argv[2];

	ensureFinalSlash(indir);
	ensureFinalSlash(outdir);

	// Process the .sfx files in the input directory

	vector<string> files = filesInDir(indir, "*.sfx");

	for (const string & inFileName : files)
	{
		string baseFileName = inFileName;
		string::size_type n = baseFileName.find_last_of('.');
		if (n != string::npos)
		{
			string ext = baseFileName.substr(n+1);
			if (ext == "sfx")
				baseFileName.erase(n);
		}
		n = baseFileName.find_last_of("/\\");
		if (n != string::npos)
		{
			baseFileName.erase(0, n+1);
		}
		string outFileName = outdir;
		outFileName += baseFileName;
		outFileName += ".wav";

		printf("%s --> %s\n", inFileName.c_str(), outFileName.c_str());

		(void) convertFile(inFileName, outFileName);
	}

	return 0;
}
