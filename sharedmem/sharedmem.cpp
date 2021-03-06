// sharedmem.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <functional>
#include <utility>
#include <stdio.h>
#include <map>
#include <sstream>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "..\include\sharedmem_classes.h"
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp> 
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>


boost::mutex io_mutex;
boost::mutex io_mutex2;
boost::mutex io_mutex3;

#ifdef WIN32
#pragma data_seg ("SHARED")
char sharedGlobalDirectoryName[512] = "c:\\temp\\";
#pragma data_seg()
#pragma comment(linker, "/section:SHARED,RWS")
#else
#pragma message("undone! need to devise shared memory for this platform")
#error dunno
char sharedGlobalDirectoryName[512] = "hello world";
#endif


//helpers for the extern "C" functions, not called directly



//end helpers


extern "C"
{


	__declspec(dllexport) void clearmap(char * sharedMapName, char * properMapName)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		using namespace boost::interprocess;
		typedef int    KeyType;
		typedef std::string  MappedType;
		typedef std::pair<const int, std::string> ValueType;

		//Alias an STL compatible allocator of for the map.
		//This allocator will allow to place containers
		//in managed shared memory segments
		typedef allocator<ValueType, managed_shared_memory::segment_manager>
			ShmemAllocator;
		typedef map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MyMap;

		managed_shared_memory segment(open_only, sharedMapName);          //segment size in bytes
		ShmemAllocator alloc_inst(segment.get_segment_manager());
		
		MyMap *mymap = segment.find<MyMap>(properMapName).first;

		mymap->clear();


		

	}


	__declspec(dllexport) void initSharedMemoryMap(char * sharedMapName)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);
		using namespace boost::interprocess;

		//Remove shared memory on construction and destruction
		//struct shm_remove
		//{ 
		//	shm_remove() { shared_memory_object::remove("MySharedMemory"); }
		//	~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
		//} remover;
		shared_memory_object::remove(sharedMapName);

	}


	__declspec(dllexport) void setSharedMemoryFileLocation(char * sharedMapFileName)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		strcpy(sharedGlobalDirectoryName, sharedMapFileName);

	}

	__declspec(dllexport) void getSharedMemoryFileLocation(char * sharedMapFileName)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		strcpy(sharedMapFileName, sharedGlobalDirectoryName);

	}


	__declspec(dllexport) void setSharedMemoryMap(char * sharedMapName, std::map<int, std::string> yourmap)
	{ 
		boost::mutex::scoped_lock lock(io_mutex);

		boost::filesystem::path dir(sharedGlobalDirectoryName);
		boost::filesystem::path file(sharedMapName);
		boost::filesystem::path full_path = dir / file;

		std::ofstream ofs(full_path.c_str());


		//std::ifstream ifs("c:\\temp\\test.xxx");

		try
		{
			boost::archive::text_oarchive oarch(ofs);
			oarch << yourmap;
			ofs.close();
		}
		catch (...)
		{
			printf("%s: location does not exist.\n", __FUNCTION__);
		}
		//try catch here would be good.


		//std::map<int, int> new_map;
		/*boost::archive::text_iarchive iarch(ifs);
		iarch >> map2;
		std::cout << map2[0] << std::endl;
*/
	}

	__declspec(dllexport) void setSharedMemoryMap2_deprecated(char * sharedMapName, std::map<int, std::string> yourmap)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		using namespace boost::interprocess;
		auto ln = yourmap.size();
	//	char temp[256];
//		sprintf(temp, "%d", ln);
	//	MessageBoxA(0, "A", temp, MB_OK);
		//Remove shared memory on construction and destruction
		//struct shm_remove
		//{ 
		//	shm_remove() { shared_memory_object::remove("MySharedMemory"); }
		//	~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
		//} remover;
		shared_memory_object::remove(sharedMapName);

		//Shared memory front-end that is able to construct objects
		//associated with a c-string. Erase previous shared memory with the name
		//to be used and create the memory segment at the specified address and initialize resources
		managed_shared_memory segment(create_only, sharedMapName , 65536);          //segment size in bytes

							   //Note that map<Key, MappedType>'s value_type is std::pair<const Key, MappedType>,
							   //so the allocator must allocate that pair.
		typedef int    KeyType;
		typedef std::string  MappedType;
		typedef std::pair<const int, std::string> ValueType;

		//Alias an STL  compatible allocator of for the map.
		//This allocator will allow to place containers
		//in managed shared memory segments
		typedef allocator<ValueType, managed_shared_memory::segment_manager>
			ShmemAllocator;

		//Alias a map of ints that uses the previous STL-like allocator.
		//Note that the third parameter argument is the ordering function
		//of the map, just like with std::map, used to compare the keys.
		typedef map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MyMap;

		//Initialize the shared memory STL-compatible allocator
		ShmemAllocator alloc_inst(segment.get_segment_manager());

		//Construct a shared memory map.
		//Note that the first parameter is the comparison function,
		//and the second one the allocator.
		//This the same signature as std::map's constructor taking an allocator
		MyMap *mymap =
			segment.construct<MyMap>(sharedMapName)      //object name
			(std::less<int>() //first  ctor parameter
				, alloc_inst);     //second ctor parameter

								   //Insert data in the map
		/*for (int i = 0; i < 100; ++i) {
			mymap->insert(std::pair<const int, float>(i, (float)i));
		}*/
		
		for (auto it = yourmap.begin(); it != yourmap.end(); it++)
		{
			mymap->insert(std::pair<const int, std::string>(it->first,  it->second));
		}
	//	mymap->insert(std::pair<const int, std::string>(0, "booger"));
	//	mymap->insert(std::pair<const int, std::string>(1, "boobs"));


	//	return 0;
	}

	__declspec(dllexport) void addToSharedMap(int index, std::string property, char * sharedMapName, char * properMapName)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		using namespace boost::interprocess;
		typedef int    KeyType;
		typedef std::string  MappedType;
		typedef std::pair<const int, std::string> ValueType;

		//Alias an STL compatible allocator of for the map.
		//This allocator will allow to place containers
		//in managed shared memory segments
		typedef allocator<ValueType, managed_shared_memory::segment_manager>
			ShmemAllocator;
		typedef map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MyMap;

		managed_shared_memory segment(open_only, sharedMapName);          //segment size in bytes
		ShmemAllocator alloc_inst(segment.get_segment_manager());

		MyMap *mymap = segment.find<MyMap>(properMapName).first;

		//(std::less<int>() 			, alloc_inst);

		(*mymap)[index] = property;


	}

	__declspec(dllexport) void getSharedMemoryMap(char * sharedMapName, std::map<int, std::string> &yourmap)
	{
		boost::mutex::scoped_lock lock(io_mutex);
		//boost::mutex::scoped_lock
		//	lock(io_mutex3);

		boost::filesystem::path dir(sharedGlobalDirectoryName);
		boost::filesystem::path file(sharedMapName);
		boost::filesystem::path full_path = dir / file;

		
		//std::ofstream ofs("c:\\temp\\test.xxx");
		std::ifstream ifs(full_path.c_str());
		//std::map<int, int> new_map;
		try
		{
			boost::archive::text_iarchive iarch(ifs);
			iarch >> yourmap;

		}
		catch (const std::exception& e)//file doesn't exist, that's ok.  MIght be the first set. Ignore. 
		{

		}

	}

	//
	__declspec(dllexport) void deletePresetArrayFromMap( PresetMapStruct ** presetArray)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		delete *presetArray;
	}


/*
	def make(pre, bank) :
		return (pre << 8) + bank

		def unmake(key) :
		return[key >> 8, key & 127]*/

	__declspec(dllexport) void setPresetArrayFromMap(char * sharedMapName, PresetMapStruct * presetArray, int size)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		std::map<int, std::string> mp;

		for (int i = 0; i < size; i++)
		{
			int key;
			key = (presetArray[i].presetID << 8) + presetArray[i].bank;
//			mp[presetArray[i].presetID] = presetArray[i].name;
			mp[key] = presetArray[i].name;

		}

		setSharedMemoryMap(sharedMapName, mp);

	}


	__declspec(dllexport) void getPresetArrayFromMap(char * sharedMapName,  PresetMapStruct ** presetArray, int *size)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		std::map<int, std::string> yourmap;
		boost::filesystem::path dir(sharedGlobalDirectoryName);
		boost::filesystem::path file(sharedMapName);
		boost::filesystem::path full_path = dir / file;

		std::istringstream iss;
		std::ostringstream oss;
		//boost::archive::text_oarchive oa(oss);
				
		//std::ofstream ofs("c:\\temp\\test.xxx");
		std::ifstream ifs(full_path.c_str());
		//std::map<int, int> new_map;
		try
		{
			boost::archive::text_iarchive iarch(ifs);
			iarch >> yourmap;

		}
		catch (const std::exception&)
		{
			*size = 0; //file doesnt exist. 
			return;
		}
		
		*size= yourmap.size();
		*presetArray = new PresetMapStruct[*size];
		int i = 0;
		int pre, bank;

		for (auto it = yourmap.begin(); it != yourmap.end(); it++)
		{

			//return[key >> 8, key & 127] 
			int bullshit = it->first;

			pre = it->first >> 8;
			bank = it->first & 127;

			//(*presetArray)[i].presetID = it->first;
			(*presetArray)[i].presetID = pre;
			(*presetArray)[i].bank = bank;
			strcpy((*presetArray)[i].name, it->second.c_str());
			i++;
		}



	}


	__declspec(dllexport) bool SFLoadedFileCheck(char * soundfont)
	{
		namespace fs = boost::filesystem;

		fs::path targetDir(sharedGlobalDirectoryName);

		fs::directory_iterator it(targetDir), eod;


		BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod))
		{
			if (fs::is_regular_file(p))
			{
				//
				std::vector<std::string> strs;
				boost::split(strs, p.filename().string(), boost::is_any_of("~"));
				//
				if (strs.size() <= 1)
					continue;

				auto sfname = strs[1];

				if (sfname == soundfont)
				{
					return true;
				}


			}

		}

		return false;
	}


	__declspec(dllexport) bool findSFFileAndIDFromName(char * soundfont, std::string &fname, int *sfID)
	{

		namespace fs = boost::filesystem;

		fs::path targetDir(sharedGlobalDirectoryName);
		fs::directory_iterator it(targetDir), eod;


		BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod))
		{
			if (fs::is_regular_file(p))
			{
				//
				std::vector<std::string> strs;
				boost::split(strs, p.filename().string(), boost::is_any_of("~"));
				//
				if (strs.size() <= 1)
					continue;

				auto sfname = strs[1];
				auto sfid = strs[2];

				if (sfname == soundfont)
				{
					*sfID = boost::lexical_cast<int>(sfid);
					fname = p.string();
					
					return true;

				}

			}
		}



		return false;
	}



	//helper that finds full name of loaded soundfont on disk

	__declspec(dllexport) bool deletePresetMulti(char * soundfont)
	{
		int sfID=0;
		std::string fname;

		if (findSFFileAndIDFromName(soundfont, fname, &sfID))
		{
			boost::filesystem::remove(boost::filesystem::path(fname));
			return true;
		}

		return false;

	}

	__declspec(dllexport) int getSFID(char * soundfont)
	{

		int sfID=0;
		std::string fname;

		if (findSFFileAndIDFromName(soundfont, fname, &sfID))
		{
			return sfID;

		}
		else
			return -1;
		

	}
	
	__declspec(dllexport) void getPresetArrayFromMapMulti(char * soundfont, PresetMapStruct ** presetArray, int *size)
	{

		namespace fs = boost::filesystem;

		fs::path targetDir(sharedGlobalDirectoryName);

		fs::directory_iterator it(targetDir), eod;
		*size = 0;

		BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod))
		{
			if (fs::is_regular_file(p))
			{
//
				std::vector<std::string> strs;
				boost::split(strs, p.filename().string(), boost::is_any_of("~"));
//
				if (strs.size() <= 1)
					continue;
				
				auto sfname = strs[1];
				auto sfid = strs[2];

				if (sfname == soundfont)
				{
					
					getPresetArrayFromMap((char*)p.filename().string().c_str(), presetArray, size);

				}

			}
		}


	}

	__declspec(dllexport) void getSharedMemoryMap2_deprecated(char * sharedMapName, std::map<int, std::string> &yourmap)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		using namespace boost::interprocess;
		typedef int    KeyType;
		typedef std::string  MappedType;
		typedef std::pair<const int, std::string> ValueType;

		//Alias an STL compatible allocator of for the map.
		//This allocator will allow to place containers
		//in managed shared memory segments
		typedef allocator<ValueType, managed_shared_memory::segment_manager>
			ShmemAllocator;
		typedef map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MyMap;

		managed_shared_memory segment(open_only, sharedMapName );          //segment size in bytes
		ShmemAllocator alloc_inst(segment.get_segment_manager());

		MyMap *mymap = segment.find<MyMap>(sharedMapName).first;

			//(std::less<int>() 			, alloc_inst);
		
		int i=0;

		for (auto it = mymap->begin(); it != mymap->end(); it++)
		{
			yourmap[it->first] = it->second;
			printf("got:%s\n", it->second.c_str());
		}

		
		//printf("got:%s\n", mymap->operator[](1).c_str());
		//(*mymap)[2] = "dunno";

		//Find the vector using the c-string name
		//MyVector *myvector = segment.find<MyVector>("MyVector").first;

	}

	__declspec(dllexport) void setSharedElement(char * sharedMapName, int index, std::string value)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex2);

		std::map<int, std::string> m;
		getSharedMemoryMap(sharedMapName, m);
		m[index] = value;
		setSharedMemoryMap(sharedMapName, m);



	}
	__declspec(dllexport) void getSharedElement(char * sharedMapName, int index, std::string &value)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		std::map<int, std::string> m;
		getSharedMemoryMap(sharedMapName, m);
		value = m[index];



	}

	__declspec(dllexport) void setSharedElement2(char * sharedMapName, int index, char * value)
	{
		//boost::mutex::scoped_lock lock(io_mutex);

		std::string s = value;
		setSharedElement(sharedMapName, index, value);

	}
	__declspec(dllexport) void getSharedElement2(char * sharedMapName, int index, char * value, int * size)
	{
		//boost::mutex::scoped_lock
		//	lock(io_mutex);

		std::string s; 

		getSharedElement(sharedMapName, index, s);
		strcpy(value, s.c_str());
		*size = s.size();


		


	}




}