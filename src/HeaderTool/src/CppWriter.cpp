
#include "CppParser.h"
#include <Util/FStream.h>

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

EProjectType ProjectType = ENGINE_DLL;

FString GeneratedOutput;
//FString config;
//FString platform;
FString targetPath;
FString projectName;

TArray<FTypeDefinition> PreRegisteredClasses;
TArray<FHeaderData> Headers;

const char* funcCommmandTypes[] = {
	"GENERAL",
	"OUTPUT",
	"COMMAND",
	"SERVER_RPC",
	"CLIENT_RPC",
	"MULTICAST_RPC"
};

bool ClassExists(const FString& name)
{
	if (name.IsEmpty())
		return false;

	for (auto& header : Headers)
	{
		for (auto Class : header.classes)
		{
			if (Class.name == name)
				return true;
		}
	}
	for (auto& t : PreRegisteredClasses)
		if (t.name == name && t.type == 1)
			return true;

	return false;
}

FString GetVariableType(const CppProperty& property, bool bIgnoreTemplate = false)
{
	if (property.bTemplateType && !bIgnoreTemplate)
	{
		if (property.templateTypename == "TArray")
			return "EVT_ARRAY";
		if (property.templateTypename == "TMap")
			return "EVT_MAP";
		if (property.templateTypename == "TObjectPtr")
			return "EVT_OBJECT_PTR";
		if (property.templateTypename == "TEnum")
			return "EVT_ENUM";
		if (property.templateTypename == "TClassPtr")
			return "EVT_CLASS_PTR";
	}
	if (bIgnoreTemplate)
	{
		if (property.nestedTemplateType == "TEnum")
			return "EVT_ENUM";
		if (property.nestedTemplateType == "TObjectPtr")
			return "EVT_OBJECT_PTR";
	}

	if (property.typeName == "FString" || property.typeName == "WString")
		return "EVT_STRING";

	if (property.typeName == "SizeType" || property.typeName == "size_t")
		return "EVT_UINT";

	if (property.typeName == "float")
		return "EVT_FLOAT";
	if (property.typeName == "double")
		return "EVT_DOUBLE";
	if (property.typeName.Find("int") != -1)
		return "EVT_INT";
	if (property.typeName.Find("uint") != -1)
		return "EVT_UINT";
	if (property.typeName == "bool")
		return "EVT_BOOL";

	if (property.typeName == "FOutput")
		return "EVT_OUTPUT";

	for (auto header : Headers)
	{
		for (auto Class : header.classes)
		{
			if (Class.name == property.typeName)
				return Class.classMacro.type == FMacro::STRUCT ? "EVT_STRUCT" : "EVT_CLASS";
		}
		for (auto Enum : header.enums)
		{
			if (Enum.name == property.typeName)
				return "EVT_ENUM";
		}
	}

	for (auto t : PreRegisteredClasses)
	{
		if (t.name == property.typeName)
			return t.type == 0 ? "EVT_STRUCT" : t.type == 1 ? "EVT_CLASS" : "EVT_ENUM";
	}

	return "EVT_END";
}

void CParser::WriteModuleCpp()
{
	std::ofstream stream((GeneratedOutput + "\\" + projectName + ".init.cpp").c_str(), std::ios::trunc | std::ios::out);
	if (!stream.is_open())
		return;

	//stream << "\n#include \"Module.h\"\n\nCModule " << projectName.c_str() << "_module(\"" << projectName.c_str() << "\");\n";

	stream << "\n#include \"Module.h\"\n\n"
		<< "CModule& GetModule_" << projectName.c_str() << "()\n{\n"
		<< "\tstatic CModule _module(\"" << projectName.c_str() << "\");\n"
		<< "\treturn _module;\n"
		<< "}\n";

	if (ProjectType != ENGINE_DLL)
		stream << std::endl << "extern \"C\" __declspec(dllexport) CModule* __GetModuleInstance() { return &GetModule_" << projectName.c_str() << "(); }\n";

	stream.close();
}

void CParser::WriteGeneratedHeader(const FHeaderData& data)
{
	FString FilePath = (GeneratedOutput + "\\" + data.FileName + ".generated.h");
	std::ofstream stream(FilePath.c_str(), std::ios::trunc | std::ios::out);
	if (!stream.is_open())
		return;

	std::cout << "Writing Generated Header '" << FilePath.c_str() << "'\n";

	stream << "\n#include \"Object/Class.h\"\n#include \"Object/ObjectMacros.h\"\n\n";

	FString fileId = projectName + "_" + data.FileName + "_h";

	for (auto c : data.classes)
	{
		stream << "#define " << fileId.c_str() << "_" << c.bodyMacro.line << "_GeneratedBody \\\n";
		for (auto p : c.Properties)
			if (p.bPrivate)
				stream << "PRIVATE_MEMBER_OFFSET_ACCESSOR(" << c.name.c_str() << ", " << p.name.c_str() << ") \\\n";

		for (auto f : c.Functions)
		{
			//if (f.bPrivate)
			//{
			//	std::cerr << "error: Function must be public '" << f.name.c_str() << "'\n";
			//	continue;
			//}

			bool bRequiresImpl = f.type != CppFunction::COMMAND && f.type != CppFunction::GENERAL && f.type != CppFunction::OUTPUT;

			stream << "DECLARE_EXEC_FUNCTION(" << f.name.c_str() << ")\\\n";
			
			if (bRequiresImpl)
			{
				stream << "DECLARE_IMPLEMENTATION(" << f.name.c_str();
				for (auto& arg : f.Arguments)
				{
					FString typeName = arg.typeName;
					if (arg.bPointer)
						typeName += '*';
					if (arg.bTemplateType)
						typeName = FString("TObjectPtr<") + arg.typeName + ">";

					stream << ", " << typeName.c_str() << " " << arg.name.c_str();
				}

				stream << ")\\\n";
			}
		}

		if (c.classMacro.type != FMacro::STRUCT)
			stream << "DECLARE_CLASS(" << c.name.c_str() << ", " << c.baseName.c_str() << ", " << projectName.c_str() << ")\n\n";
		else
			stream << "DECLARE_STRUCT(" << c.name.c_str() << ", " << projectName.c_str() << ")\n\n";
	}

	stream << "\n#undef FILE_ID\n#define FILE_ID " << fileId.c_str() << "\n";

	stream.close();
}

static FString GetDisplayName(const FString& in)
{
	if (in.Size() < 2)
		return in;

	FString r = in;
	if (((r[0] == 'b' || r[0] == 'm' || r[0] == 'g' || r[0] == 'C' || r[0] == 'F' || r[0] == 'I') && r[1] < 'a'))
		r.Erase(r.begin());
	else if ((r[0] == 'm' && r[1] == '_'))
		r.Erase(r.begin(), r.begin() + 1);
	else if (r[0] > 'Z')
		r[0] -= 32;

	for (auto it = r.rbegin()++; it != r.rend(); it++)
	{
		if (*it > 'Z' && *(it.ptr + 1) < 'a')
		{
			SizeType ind = r.Index(it);
			r.Insert(' ', ind + 1);
			it = (TReverseIterator<char>)r.At(ind);
		}
	}

	return r;
}

void CParser::WriteGeneratedCpp(const FHeaderData& data)
{
	std::ofstream stream((GeneratedOutput + "\\" + data.FileName + ".generated.cpp").c_str(), std::ios::trunc | std::ios::out);
	if (!stream.is_open())
		return;

	FString moduleGetter = FString("GetModule_") + projectName + "()";

	stream << "\n#include <Util/Core.h>\n#include \"" << data.FilePath.c_str() << "\"\n#include \"Object/Class.h\"\n#include \"Module.h\"\n";
	//stream << "\nextern CModule " << projectName.c_str() << "_module;\n\n";
	stream << "\nCModule& " << moduleGetter.c_str() << ";\n\n";

	for (auto& Enum : data.enums)
	{
		stream	<< "class FEnum_" << Enum.name.c_str() << " : public FEnum\n{\n\tpublic:\n";
		stream << "\tFEnum_" << Enum.name.c_str() << "()\n\t{\n";

		for (auto& var : Enum.Values)
		{
			FString name;
			if (auto i = var.meta.ArgIndex("Name"); i != -1)
				name = var.meta.Arguments[i].Value;
			else
				name = var.name;

			stream << "\t\tvalues.Add({ \"" << name.c_str() << "\", (int64)" << Enum.name.c_str() << "::" << var.name.c_str() << " });\n";
		}

		FString name;
		if (auto i = Enum.macro.ArgIndex("Name"); i != -1)
			name = Enum.macro.Arguments[i].Value;
		else
		{
			name = Enum.name;
			if (name[0] == 'E' && name[1] < 'a')
				name.Erase(name.begin());
		}

		//stream  << "\t\t};\n";
		stream << "\t\tname = \"" << name.c_str() << "\";\n"
			<< "\t\tcppName = \"" << Enum.name.c_str() << "\";\n"
			<< "\t\tsize = sizeof(" << Enum.name.c_str() << ");\n"
			<< "\t\tflags = EnumFlag_NONE";

		for (auto& mf : Enum.macro.Arguments)
		{
			if (mf.Key == "IsFlag")
				stream << " | EnumFlag_IS_FLAG";
		}

		stream << ";\n";
		stream << "\t\t" << moduleGetter.c_str() << ".RegisterFEnum(this);\n\t}\n};\n";
		stream << "FEnum_" << Enum.name.c_str() << " __FEnum_" << Enum.name.c_str() << "_Instance;\n\n";
	}

	for (auto Class : data.classes)
	{
		stream << "#undef CLASS_NEXT_PROPERTY\n#define CLASS_NEXT_PROPERTY nullptr\n\n";

		for (auto p : Class.Properties)
		{
			FString varTypeId = GetVariableType(p);
			if (varTypeId == "EVT_END")
			{
				std::cerr << "WARNING: invalid property '" << Class.name.c_str() << "::" << p.name.c_str() << "'!\n";
				continue;
			}
			if (!p.IfGuard.IsEmpty())
				stream << "#if " << p.IfGuard.c_str() << "\n";

			if (varTypeId == "EVT_ARRAY")
			{
				FString ArrayTypeName = p.fullTypename;
				/*if (p.nestedTemplateType.IsEmpty())
					ArrayTypeName += p.typeName + (p.bPointer ? "*" : "") + ">";
				else
					ArrayTypeName += p.nestedTemplateType + "<" + p.typeName + (p.bPointer ? "*" : "") + ">>";*/

				FString objTypeName;
				if (p.nestedTemplateType.IsEmpty())
					objTypeName = p.typeName + (p.bPointer ? "*" : "");
				else
					objTypeName = p.nestedTemplateType + "<" + p.typeName + (p.bPointer ? "*" : "") + ">";

				FString type = GetVariableType(p, true);
				stream << "static FArrayHelper _arrayHelper_" << p.name.c_str() << "{\n ";
				stream << "\t[](void* ptr) { (*(" << ArrayTypeName.c_str() << "*)ptr).Add(" << (p.bPointer ? FString(");") : (type == "EVT_OBJECT_PTR" ? ");" : p.typeName + "());")).c_str() << " },\n";
				stream << "\t[](void* ptr, SizeType i) { (*(" << ArrayTypeName.c_str() << "*)ptr).Erase(" << "(*(" << ArrayTypeName.c_str() << "*)ptr).At(i)); },\n";
				stream << "\t[](void* ptr) { (*(" << ArrayTypeName.c_str() << "*)ptr).Clear(); },\n";
				stream << "\t[](void* ptr) { return (*(" << ArrayTypeName.c_str() << "*)ptr).Size(); }, \n";
				stream << "\t[](void* ptr) { return (void*)(*(" << ArrayTypeName.c_str() << "*)ptr).Data(); }, \n";
				stream << "\t" << type.c_str() << ", \n";
				stream << "\tsizeof(" << objTypeName.c_str() << ")\n};\n\n";
			}

			// MetaData
			bool bHasMeta = false;
			FString metaName = "_" + Class.name + "_" + p.name + "_Meta";
			{
				auto uiMinI = p.macro.ArgIndex("UIMin");
				auto uiMaxI = p.macro.ArgIndex("UIMax");
				auto catI = p.macro.ArgIndex("Category");

				bHasMeta = uiMinI != -1 || uiMaxI != -1 || catI != -1 || p.macro.Arguments.Size() != 0;
				if (bHasMeta)
				{
					stream << "#if IS_DEV\n";

					stream << "static TPair<FString, FString> " << metaName.c_str() << "_Tags[]" << "{\n";
					for (auto& arg : p.macro.Arguments)
						stream << "\t{ \"" << arg.Key.c_str() << "\", \"" << arg.Value.c_str() << "\" },\n";

					stream << "};\n\n";

					stream << "static FPropertyMeta " << metaName.c_str() << " {\n";
					stream << "\t\"" << (uiMinI != -1 ? p.macro.Arguments[uiMinI].Value.c_str() : "") << "\",\n";
					stream << "\t\"" << (uiMaxI != -1 ? p.macro.Arguments[uiMaxI].Value.c_str() : "") << "\",\n";
					stream << "\t\"" << (catI != -1 ? p.macro.Arguments[catI].Value.c_str() : "") << "\",\n";
					stream << "\t\"\",\n";

					stream << "\t" << std::to_string(p.macro.Arguments.Size()) << ",\n";
					stream << "\t" << metaName.c_str() << "_Tags\n";
					
					/*if (onEditI != -1)
					{
						FString func = p.macro.Arguments[onEditI].Value;
						CppFunction* f = Class.GetFunction(func);
						if (!f)
						{
							std::cerr << "error: unkown function '" << func.c_str() << "'!\n";
							stream << "\tnullptr\n";
						}
						else if (f->Arguments.Size() > 0)
						{
							std::cerr << "error: OnEditFunc cannot take in arguments!\n";
							stream << "\tnullptr\n";
						}
						else
						{
							stream << "\t&" << Class.name.c_str() << "::exec" << func.c_str() << "\n";
						}
					}
					else
						stream << "\tnullptr\n";*/

					stream << "};\n\n";

					stream << "#define " << metaName.c_str() << "_Ptr &" << metaName.c_str() << "\n";
					stream << "#else\n";
					stream << "#define " << metaName.c_str() << "_Ptr nullptr\n";
					stream << "#endif\n";
				}
			}

			FString displayName;
			SizeType dnI = p.macro.ArgIndex("Name");
			if (dnI == -1)
				displayName = GetDisplayName(p.name);
			else
				displayName = p.macro.Arguments[dnI].Value;

			FString tags;
			if (p.bPointer)
				tags += " VTAG_TYPE_POINTER |";
			if (SizeType i = p.macro.ArgIndex("EditorVisible"); i != -1)
				tags += " VTAG_EDITOR_VISIBLE |";
			if (SizeType i = p.macro.ArgIndex("Editable"); i != -1)
				tags += " VTAG_EDITOR_EDITABLE |";
			if (SizeType i = p.macro.ArgIndex("DontSerialize"); i == -1)
				tags += " VTAG_SERIALIZABLE |";
			if (p.bStatic)
				tags += " VTAG_STATIC |";

			if (!tags.IsEmpty())
				tags.Erase(tags.last());
			else
				tags = " VTAG_NONE";

			stream << "DECLARE_PROPERTY(" << Class.name.c_str() << ", \"" 
				<< displayName.c_str() << "\", "
				<< p.name.c_str() << ", \""
				<< p.comment.c_str() << "\", \""
				<< p.typeName.c_str() << "\", "
				<< varTypeId.c_str() << ","
				<< tags.c_str() << ", ";

			if (p.bPrivate)
				stream << Class.name.c_str() << "::__private_" << p.name.c_str() << "_offset(), ";
			else
				stream << "offsetof(" << Class.name.c_str() << ", " << p.name.c_str() << "), ";
			
			stream << "sizeof(" << p.fullTypename.c_str();
			stream << "), ";

			stream << (bHasMeta ? (metaName + "_Ptr, ").c_str() : "nullptr, ");

			if (varTypeId == "EVT_ARRAY")
				stream << "&_arrayHelper_" << p.name.c_str();
			else
				stream << "nullptr";
			
			stream << ")\n";

			stream << "#undef CLASS_NEXT_PROPERTY\n"
				   << "#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(" << Class.name.c_str() << ", " << p.name.c_str() << ")\n";

			if (!p.IfGuard.IsEmpty())
				stream << "#endif\n";

			stream << "\n";
		}

		if (Class.classMacro.type != FMacro::STRUCT)
			stream << "#undef CLASS_NEXT_FUNCTION\n#define CLASS_NEXT_FUNCTION nullptr\n\n";

		for (auto& f : Class.Functions)
		{
			FString displayName = f.name;
			SizeType dnI = f.macro.ArgIndex("Name");
			if (dnI == -1)
			{
				if (displayName[0] > 'Z')
					displayName[0] -= 32;
			}
			else
				displayName = f.macro.Arguments[dnI].Value;

			FString CmdType = funcCommmandTypes[f.type];

			if (f.Arguments.Size() > 0)
			{
				stream << "static FFuncArg _funcArgs_" << Class.name.c_str() << "_" << f.name.c_str() << "[] = {\n";

				for (auto& arg : f.Arguments)
				{
					FString typeId = GetVariableType(arg);
					if (typeId == "EVT_END" || typeId == "EVT_ARRAY" || typeId == "EVT_MAP")
					{
						std::cerr << "ERROR: invalid function arg '" << arg.name.c_str() << "'  '" << Class.name.c_str() << "::" << f.name.c_str() << "'!\n";
						continue;
					}

					FString flags = "VTAG_NONE";
					if (arg.bPointer)
						flags += " | VTAG_TYPE_POINTER";

					FString objType = "nullptr";
					if (typeId == "EVT_CLASS")
						objType = arg.typeName + "::StaticClass()";
					else if (typeId == "EVT_STRUCT")
						objType = arg.typeName + "::StaticStruct()";

					stream << "\t{ \"" << arg.name.c_str() << "\", " << typeId.c_str() << ", " << flags.c_str() << ", " << objType.c_str() << " },\n";
				}

				stream << "};\n\n";
			}

			stream << "DECLARE_FUNCTION_PROPERTY("
				<< Class.name.c_str() << ", \""
				<< displayName.c_str() << "\", \""
				<< f.comment.c_str() << "\", "
				<< f.name.c_str() << ", "
				<< "&" << Class.name.c_str() << "::exec" << f.name.c_str() << ", "
				<< "FFunction::" << CmdType.c_str();
			if (f.Arguments.Size() > 0)
				stream << ", _funcArgs_" << Class.name.c_str() << "_" << f.name.c_str() << ", " << f.Arguments.Size() << ", ";
			else
				stream << ", nullptr, 0, ";

			//for (auto& arg : f.Arguments)
			//{
			//	FString typeId = GetVariableType(arg);
			//	if (typeId == "EVT_END" || typeId == "EVT_ARRAY" || typeId == "EVT_MAP")
			//	{
			//		std::cerr << "ERROR: invalid function arg '" << arg.name.c_str() << "'  '" << Class.name.c_str() << "::" << f.name.c_str() << "'!\n";
			//		continue;
			//	}

			//	FString flags = "VTAG_NONE";
			//	if (arg.bPointer)
			//		flags += " | VTAG_TYPE_POINTER";

			//	FString objType = "nullptr";
			//	if (typeId == "EVT_CLASS")
			//		objType = arg.typeName + "::StaticClass()";
			//	else if (typeId == "EVT_STRUCT")
			//		objType = arg.typeName + "::StaticStruct()";

			//	stream << "{ \"" << arg.name.c_str() << "\", " << typeId.c_str() << ", " << flags.c_str() << ", " << objType.c_str() << " }, ";
			//}

			FString funcFlags = "FunctionFlags_NONE";
			if (f.macro.ArgIndex("NoEntityInput") == -1)
				funcFlags += " | FunctionFlags_ALLOW_AS_INPUT";

			stream << (f.bStatic ? "1" : "0") << ", " << funcFlags.c_str() << ")\n";
			stream << "#undef CLASS_NEXT_FUNCTION\n" << "#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(" << Class.name.c_str() << ", " << f.name.c_str() << ")\n\n";
		}

		FString objectTypeName;
		if (Class.classMacro.type == FMacro::CLASS)
			objectTypeName = FString("FClass_") + Class.name;
		else if (Class.classMacro.type == FMacro::STRUCT)
			objectTypeName = FString("FStruct_") + Class.name;
		else
			objectTypeName = FString("FAssetClass_") + Class.name;

		bool bHasTags = Class.classMacro.Arguments.Size() > 0;
		if (bHasTags)
		{
			stream << "#ifdef IS_DEV\n";
			stream << "static TPair<FString, FString> _" << objectTypeName.c_str() << "_Tags[] {\n";
			for (auto& arg : Class.classMacro.Arguments)
				stream << "\t{ \"" << arg.Key.c_str() << "\", \"" << arg.Value.c_str() << "\" },\n";
			stream << "};\n";
			stream << "#endif\n\n";
		}

		if (Class.classMacro.type == FMacro::CLASS)
			stream << "class " << objectTypeName.c_str() << " : public FClass\n{\n";
		else if (Class.classMacro.type == FMacro::STRUCT)
			stream << "class " << objectTypeName.c_str() << " : public FStruct\n{\n";
		else
			stream << "class " << objectTypeName.c_str() << " : public FAssetClass\n{\n";

		FString displayName;
		if (SizeType dnI = Class.classMacro.ArgIndex("Name"); dnI == -1)
			displayName = GetDisplayName(Class.name);
		else
			displayName = Class.classMacro.Arguments[dnI].Value;

		stream << "public:\n\t"
			<< objectTypeName.c_str() << "()\n\t{\n"
			<< "\t\tname = \"" << displayName.c_str() << "\";\n"
			<< "\t\tcppName = \"" << Class.name.c_str() << "\";\n"
			<< "\t\tsize = sizeof(" << Class.name.c_str() << ");\n"
			<< "\t\tnumProperties = " << std::to_string(Class.Properties.Size()) << ";\n"
			<< "\t\tPropertyList = CLASS_NEXT_PROPERTY;\n"
			<< "\t\tbIsClass = " << (Class.classMacro.type != FMacro::STRUCT ? "true" : "false") << ";\n";

		if (bHasTags)
		{
			stream << "#ifdef IS_DEV\n";
			stream << "\t\tnumTags = " << std::to_string(Class.classMacro.Arguments.Size()) << ";\n";
			stream << "\t\ttags = _" << objectTypeName.c_str() << "_Tags;\n";
			stream << "#endif\n";
		}

		if (auto i = Class.classMacro.ArgIndex("Extension"); Class.classMacro.type == FMacro::ASSET && Class.classMacro.ArgIndex("Abstract") == -1)
		{
			if (i == -1)
				std::cerr << "error: type of asset must have extension type specified!";
			else
				stream << "\t\textension = \"" << Class.classMacro.Arguments[i].Value.c_str() << "\";\n";

			auto importI = Class.classMacro.ArgIndex("ImportableAs");
			if (importI != -1)
				stream << "\t\timportableAs = \"" << Class.classMacro.Arguments[importI].Value.c_str() << "\";\n";
		}

		if (Class.classMacro.type != FMacro::STRUCT)
		{
			//bool bBaseClass = ClassExists(Class.baseName);
			bool bBaseClass = Class.name != "CObject";
			stream << "\t\tBaseClass = " << (bBaseClass ? (Class.baseName + "::StaticClass()").c_str() : "nullptr") << ";\n"
				<< "\t\tnumFunctions = " << std::to_string(Class.Functions.Size()) << ";\n"
				<< "\t\tFunctionList = CLASS_NEXT_FUNCTION;\n"
				<< "\t\tflags =";

			FString flags = "";

			if (Class.classMacro.ArgIndex("Abstract") != -1)
				flags += " CTAG_ABSTRACT |";
			if (Class.classMacro.ArgIndex("Hidden") != -1)
				flags += " CTAG_HIDDEN |";
			if (Class.classMacro.ArgIndex("Static") != -1)
				flags += " CTAG_STATIC |";

			if (!flags.IsEmpty())
				flags.Erase(flags.last() - 1, flags.end());
			else
				flags = " CTAG_NONE";

			stream << flags.c_str() << ";\n";

			if (Class.classMacro.type == FMacro::ASSET)
			{
				stream << "\t\tassetFlags = ASSET_NONE";
				FString aFlags;
				if (Class.classMacro.ArgIndex("AutoLoad") != -1)
					aFlags += " | ASSET_AUTO_LOAD";

				stream << aFlags.c_str() << ";\n";
			}

			bool bCanInstantiate = Class.classMacro.ArgIndex("Abstract") == -1;
			
			if (Class.classMacro.type == FMacro::CLASS || Class.classMacro.type == FMacro::ASSET)
				stream << "\t\t" << moduleGetter.c_str() << ".RegisterFClass(this);\n";
			if (Class.classMacro.type == FMacro::ASSET)
				stream << "\t\t" << moduleGetter.c_str() << ".RegisterFAsset(this);\n";

			stream << "\t}\n\tCObject* Instantiate() override { return " << (bCanInstantiate ? (FString("new ") + Class.name + "()").c_str() : "nullptr") << "; }\n};\n";
		}
		else
			stream << "\t\t" << moduleGetter.c_str() << ".RegisterFStruct(this);\n\t}\n};\n";

		stream << objectTypeName.c_str() << " __" << objectTypeName.c_str() << "_Instance;\n";

		if (Class.classMacro.type != FMacro::STRUCT)
			stream << "\nFClass* " << Class.name.c_str() << "::StaticClass() { return &__" << objectTypeName.c_str() << "_Instance; }\n";
		else
			stream << "\nFStruct* " << Class.name.c_str() << "::StaticStruct() { return &__" << objectTypeName.c_str() << "_Instance; }\n";

		stream << "\n#undef CLASS_NEXT_PROPERTY\n#undef CLASS_NEXT_FUNCTION\n";
	}

	for (auto Class : data.classes)
	{
		for (auto& f : Class.Functions)
		{
			bool bIsNetFunc = f.type == CppFunction::SERVER_RPC || f.type == f.type == CppFunction::CLIENT_RPC || f.type == CppFunction::MUTLICAST_RPC;
			bool bHasImpl = bIsNetFunc;

			FString displayName;
			if (SizeType dnI = f.macro.ArgIndex("Name"); dnI == -1)
				displayName = GetDisplayName(f.name);
			else
				displayName = f.macro.Arguments[dnI].Value;

			if (bIsNetFunc)
			{

			}

			if (f.type == CppFunction::OUTPUT)
			{
				stream << "\nvoid " << Class.name.c_str() << "::" << f.name.c_str() << "()\n{\n\tFireOutput(\"" << displayName.c_str() << "\");\n}\n";
			}

			stream << "\nvoid " << Class.name.c_str() << "::exec" << f.name.c_str() << "(CObject* obj, FStack& stack)\n{\n";
			for (auto it = f.Arguments.rbegin(); it != f.Arguments.rend(); it++)
			{
				FString typeName;
				if (it->bTemplateType)
					typeName = it->templateTypename + "<" + it->typeName + ">";
				else
					typeName = it->typeName;

				if (it->bPointer)
					typeName += "*";

				stream << "\tPOP_STACK_VARIABLE(" << typeName.c_str() << ", " << it->name.c_str() << ");\n";
			}
			stream << "\t((" << Class.name.c_str() << "*)obj)->" << f.name.c_str();
			if (bHasImpl)
				stream << "_Implementation";
			stream << "(";
			FString finalArgs;
			for (auto& arg : f.Arguments)
				finalArgs += arg.name + ",";
			if (finalArgs.Size() > 0)
				finalArgs.Erase(finalArgs.last());

			stream << finalArgs.c_str() << ");\n}\n";
		}
	}

	stream.close();
}

void CParser::LoadModuleData(const FString& path)
{
	CFStream stream((path + "/module.bin").c_str(), "rb");
	if (!stream.IsOpen())
	{
		std::cerr << "error: failed to open '" << path.c_str() << "Intermediate\\module.bin'";
		return;
	}

	FString moduleName;
	stream >> moduleName;

	SizeType numClasses;
	stream >> &numClasses;

	for (SizeType i = 0; i < numClasses; i++)
	{
		FTypeDefinition type;
		type.moduleName = moduleName;
		
		stream >> &type.type;
		stream >> type.name;

		PreRegisteredClasses.Add(type);
	}
}

bool CParser::WriteModuleData()
{
	CFStream stream((targetPath + "\\Intermediate\\module.bin").c_str(), "wb");
	if (!stream.IsOpen())
		return false;

	SizeType numClasses = 0;
	for (auto& header : Headers)
	{
		numClasses += header.classes.Size();
		numClasses += header.enums.Size();
	}

	stream << projectName;
	stream << &numClasses;

	for (auto& header : Headers)
	{
		for (auto c : header.classes)
		{
			uint8 type = c.classMacro.type == FMacro::CLASS;
			stream << &type;
			stream << c.name;
		}
		for (auto e : header.enums)
		{
			uint8 type = 2;
			stream << &type;
			stream << e.name;
		}
	}

	return true;
}

bool CParser::HeaderUpToDate(FHeaderData& header)
{
	CFStream stream((GeneratedOutput + "\\Timestamp.bin"), "rb");
	if (!stream.IsOpen())
		return false;

	SizeType numFiles = 0;
	stream >> &numFiles;

	for (SizeType i = 0; i < numFiles; i++)
	{
		FString fileName;
		stream >> fileName;

		std::filesystem::file_time_type time;
		stream >> &time;

		if (fileName != header.FileName)
			continue;

		auto nowtime = std::filesystem::last_write_time(header.FilePath.c_str());
		if (nowtime > time)
			return false;

		return true;
	}

	return false;
}

void CParser::WriteTimestamp()
{
	CFStream stream((GeneratedOutput + "\\Timestamp.bin"), "wb");
	if (!stream.IsOpen())
		return;

	SizeType numFiles = Headers.Size();
	stream << &numFiles;

	for (auto& h : Headers)
	{
		stream << h.FileName;

		auto time = std::filesystem::last_write_time(h.FilePath.c_str());
		stream << &time;
	}
}
