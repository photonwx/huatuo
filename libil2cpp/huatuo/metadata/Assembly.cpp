
#include "Assembly.h"

#include <cstring>
#include <iostream>

#include "os/File.h"
#include "utils/MemoryMappedFile.h"
#include "vm/Assembly.h"
#include "vm/Image.h"
#include "vm/Class.h"
#include "vm/String.h"


#include "Image.h"
#include "MetadataModule.h"
#include "MetadataUtil.h"


#if IL2CPP_BYTE_ORDER != IL2CPP_LITTLE_ENDIAN
#error "only support litten endian"
#endif

using namespace il2cpp;

namespace huatuo
{
namespace metadata
{

    bool GetMappedFileBuffer(const char* assemblyFile, void*& buf, uint64_t& fileLength)
    {
        int err = 0;
        os::FileHandle* fh = os::File::Open(assemblyFile, FileMode::kFileModeOpen, FileAccess::kFileAccessRead, FileShare::kFileShareReadWrite, 0, &err);

        if (err != 0)
        {
            //utils::Logging::Write("ERROR: Could not open %s", assemblyFile);
            return false;
        }

        fileLength = os::File::GetLength(fh, &err);
        if (err != 0)
        {
            //utils::Logging::Write("ERROR: GetLength %s, err:%d", assemblyFile, err);
            os::File::Close(fh, &err);
            return false;
        }

        buf = utils::MemoryMappedFile::Map(fh);

        os::File::Close(fh, &err);
        if (err != 0)
        {
            //utils::Logging::Write("ERROR: Close %s, err:%ulld", assemblyFile, err);
            utils::MemoryMappedFile::Unmap(buf);
            buf = NULL;
            return false;
        }
        return true;
    }

    Il2CppAssembly* Assembly::LoadFromFile(const char* assemblyFile)
    {
        void* fileBuffer;
        uint64_t fileLength;
        if (!GetMappedFileBuffer(assemblyFile, fileBuffer, fileLength))
        {
            return nullptr;
        }

        return LoadFromBytes((const byte*)fileBuffer, fileLength, false);
    }

    Il2CppAssembly* Assembly::LoadFromBytes(const void* assemblyData, uint64_t length, bool copyData)
    {
        auto ass = Create((const byte*)assemblyData, length, copyData);
        vm::Assembly::Register(ass);
        return ass;
    }

    Il2CppAssembly* Assembly::Create(const byte* assemblyData, uint64_t length, bool copyData)
    {
        if (!assemblyData)
        {
            il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentNullException("rawAssembly is null"));
        }

        uint32_t imageId = MetadataModule::AllocImageIndex();
        if (imageId > kMaxLoadImageCount)
        {
            vm::Exception::Raise(vm::Exception::GetArgumentException("exceed max image index", ""));
        }
        Image* image = new Image(imageId);
        
        if (copyData)
        {
            byte* newAssebmlyData = (byte*)IL2CPP_MALLOC(length);
            std::memcpy(newAssebmlyData, assemblyData, length);
            assemblyData = newAssebmlyData;
        }
        LoadImageErrorCode err = image->Load(assemblyData, (size_t)length);


        if (err != LoadImageErrorCode::OK)
        {
            if (copyData)
            {
                IL2CPP_FREE((void*)assemblyData);
            }
            char errMsg[300];
            int strLen = snprintf(errMsg, sizeof(errMsg), "err:%d", (int)err);
            vm::Exception::Raise(vm::Exception::GetBadImageFormatException(errMsg));
            // when load a bad image, mean a fatal error. we don't clean image on purpose.
        }

        auto ass = new Il2CppAssembly{};
        auto image2 = new Il2CppImage{};

        image->InitBasic(image2);
        image->BuildIl2CppAssembly(ass);
        ass->image = image2;

        image->BuildIl2CppImage(image2);
        image2->name = ConcatNewString(ass->aname.name, ".dll");
        image2->nameNoExt = ass->aname.name;
        image2->assembly = ass;

        image->InitRuntimeMetadatas();

        return ass;
    }

}
}

