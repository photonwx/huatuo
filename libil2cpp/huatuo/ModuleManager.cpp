#include "ModuleManager.h"

#include "vm/Exception.h"
#include "vm/String.h"
#include "vm/Assembly.h"
#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/Reflection.h"
#include "icalls/mscorlib/System.Reflection/Assembly.h"
#include "icalls/mscorlib/System.Reflection/MonoCMethod.h"

#include "interpreter/InterpreterModule.h"
#include "metadata/MetadataModule.h"
#include "transform/Transform.h"

using namespace il2cpp;

namespace huatuo
{

    void PostInit()
    {

    }

	void ModuleManager::Initialize()
	{
		metadata::MetadataModule::Initialize();
		interpreter::InterpreterModule::Initialize();
        PostInit();
	}
}