
#include <string>
#include <cstdlib>
#include <cerrno>
#include "Variant.h"

FVariant::FVariant(const FVariant& o) : type(o.type)
{
	switch (type)
	{
	case INTEGER:
		uInteger = o.uInteger;
		break;
	case BOOL:
		uBool = o.uBool;
		break;
	case FLOAT:
		uFloat = o.uFloat;
		break;
	case DOUBLE:
		uDouble = o.uDouble;
		break;
	case SIZETYPE:
		uSizeType = o.uSizeType;
		break;
	case OBJ_POINTER:
		uObjPointer = o.uObjPointer;
		break;
	case VECTOR:
		new (&uVector) FVector(o.uVector);
		break;
	case QUAT:
		new (&uQuat) FQuaternion(o.uQuat);
		break;
	case COLOR:
		new (&uColor) FColor(o.uColor);
		break;
	case STRING:
		new (&uString) FString(o.uString);
		break;
	case CLASS:
		uClass = o.uClass;
		break;
	case BYTE_ARRAY:
		new (&uByteArray) TArray<uint8>(o.uByteArray);
		break;
	case STRING_ARRAY:
		new (&uStringArray) TArray<FString>(o.uStringArray);
		break;
	}
}

FVariant::FVariant(int v) : uInteger(v), type(INTEGER) {}

FVariant::FVariant(bool v) : uBool(v), type(BOOL) {}

FVariant::FVariant(float v) : uFloat(v), type(FLOAT) {}

FVariant::FVariant(double v) : uDouble(v), type(DOUBLE) {}

FVariant::FVariant(SizeType v) : uSizeType(v), type(SIZETYPE) {}

FVariant::FVariant(CObject* v) : uObjPointer(v ? v->Id() : 0), type(OBJ_POINTER) {}

FVariant::FVariant(const FVector& v) : uVector(v), type(VECTOR) {}

FVariant::FVariant(const FQuaternion& v) : uQuat(v), type(QUAT) {}

FVariant::FVariant(const FColor& v) : uColor(v), type(COLOR) {}

FVariant::FVariant(const FString& v) : uString(v), type(STRING) {}

FVariant::FVariant(const char* v) : uString(v), type(STRING) {}

FVariant::FVariant(FClass* v) : uClass(v), type(CLASS) {}

FVariant::FVariant(const TArray<uint8>& v) : uByteArray(v), type(BYTE_ARRAY) {}

FVariant::FVariant(const TArray<FString>& v) : uStringArray(v), type(STRING_ARRAY) {}

FVariant::~FVariant()
{
	switch (type)
	{
	case STRING:
		uString.~FString();
	}
}

int FVariant::AsInt() const
{
	if (type == INTEGER)
		return uInteger;

	return 0;
}

bool FVariant::AsBool() const
{
	if (type == BOOL)
		return uBool;

	return false;
}

float FVariant::AsFloat() const
{
	if (type == FLOAT)
		return uFloat;

	return 0.0f;
}

double FVariant::AsDouble() const
{
	if (type == DOUBLE)
		return uDouble;

	return 0.0;
}

SizeType FVariant::AsSizeType() const
{
	if (type == SIZETYPE)
		return uSizeType;

	return 0;
}

CObject* FVariant::AsObjectPointer() const
{
	if (type == OBJ_POINTER)
		return CObjectManager::FindObject(uObjPointer);

	return nullptr;
}

FVector FVariant::AsVector() const
{
	if (type == VECTOR)
		return uVector;

	return FVector();
}

FQuaternion FVariant::AsQuat() const
{
	if (type == QUAT)
		return uQuat;

	return FQuaternion();
}

FColor FVariant::AsColor() const
{
	if (type == COLOR)
		return uColor;

	return FColor();
}

FString FVariant::AsString() const
{
	if (type == STRING)
		return uString;

	return FString();
}

FClass* FVariant::AsClass() const
{
	if (type == CLASS)
		return uClass;

	return nullptr;
}

TArray<uint8> FVariant::AsByteArray()
{
	if (type == BYTE_ARRAY)
		return uByteArray;

	return TArray<uint8>();
}

TArray<FString> FVariant::AsStringArray()
{
	if (type == STRING_ARRAY)
		return uStringArray;

	return TArray<FString>();
}

FString FVariant::ToString() const
{
	switch (type)
	{
	case INTEGER:
		return FString::ToString(uInteger);
	case BOOL:
		return uBool ? "true" : "false";
	case FLOAT:
		return std::to_string(uFloat).c_str();
	case DOUBLE:
		return std::to_string(uDouble).c_str();
	case SIZETYPE:
		return FString::ToString(uSizeType);
	case OBJ_POINTER:
		return "CObject(" + FString::ToString(uObjPointer) + ")";
	case VECTOR:
		return ("FVector(" + std::to_string(uVector.x) + ", " + std::to_string(uVector.y) + ", " + std::to_string(uVector.z) + ")").c_str();
	case QUAT:
		return ("FQuat(" + std::to_string(uQuat.x) + ", " + std::to_string(uQuat.y) + ", " + std::to_string(uQuat.z) + ", " + std::to_string(uQuat.w) + ")").c_str();
	case COLOR:
		return ("FColor(" + std::to_string(uColor.r) + ", " + std::to_string(uColor.g) + ", " + std::to_string(uColor.b) + ", " + std::to_string(uColor.a) + ")").c_str();
	case STRING:
		return uString;
	case CLASS:
		return uClass ? uClass->GetInternalName() : FString();
	case BYTE_ARRAY:
		{
			FString r = "FByteArray{";
			for (int i = 0; i < uByteArray.Size(); i++)
			{
				char buff[24];
				std::sprintf(buff, "0x%x", uByteArray[i]);
				r += buff;
				if (i + 1 < uByteArray.Size())
					r += ", ";
			}
			r += "}";
			return r;
		}
		break;
	case STRING_ARRAY:
		{
			FString r = "FStringArray{";
			for (int i = 0; i < uStringArray.Size(); i++)
			{
				r += "\"" + uStringArray[i] + "\"";
				if (i + 1 < uByteArray.Size())
					r += ", ";
			}
			r += "}";
			return r;
		}
		break;
	}

	return FString();
}

FVariant FVariant::FromString(const FString& inStr)
{
	FVariant out(0);

	if (inStr.IsEmpty())
		return out;

	if (inStr == "true")
	{
		out.type = BOOL;
		out.uBool = true;
		return out;
	}
	if (inStr == "false")
	{
		out.type = BOOL;
		out.uBool = false;
		return out;
	}

	if (inStr.Find("CObject(") == 0)
	{
		FString inner = inStr;
		inner.Erase(inner.begin(), inner.begin() + inner.Find("(") + 1);
		inner.EraseAll(')');
		inner.EraseAll(' ');
		if (!inner.IsEmpty())
		{
			long long v = std::stoll(std::string(inner.c_str()));
			out.type = OBJ_POINTER;
			out.uObjPointer = (SizeType)v;
			return out;
		}
	}

	if (inStr.Find("FVector(") == 0)
	{
		FString inner = inStr;
		inner.Erase(inner.begin(), inner.begin() + inner.Find("(") + 1);
		inner.EraseAll(')');
		inner.EraseAll(' ');
		auto vals = inner.Split(',');
		if (vals.Size() >= 3)
		{
			float x = std::stof(std::string(vals[0].c_str()));
			float y = std::stof(std::string(vals[1].c_str()));
			float z = std::stof(std::string(vals[2].c_str()));
			new (&out.uVector) FVector(x, y, z);
			out.type = VECTOR;
			return out;
		}
	}

	if (inStr.Find("FQuat(") == 0)
	{
		FString inner = inStr;
		inner.Erase(inner.begin(), inner.begin() + inner.Find("(") + 1);
		inner.EraseAll(')');
		inner.EraseAll(' ');
		auto vals = inner.Split(',');
		if (vals.Size() >= 4)
		{
			float x = std::stof(std::string(vals[0].c_str()));
			float y = std::stof(std::string(vals[1].c_str()));
			float z = std::stof(std::string(vals[2].c_str()));
			float w = std::stof(std::string(vals[3].c_str()));
			new (&out.uQuat) FQuaternion(x, y, z, w);
			out.type = QUAT;
			return out;
		}
	}

	if (inStr.Find("FColor(") == 0)
	{
		FString inner = inStr;
		inner.Erase(inner.begin(), inner.begin() + inner.Find("(") + 1);
		inner.EraseAll(')');
		inner.EraseAll(' ');
		auto vals = inner.Split(',');
		if (vals.Size() >= 3)
		{
			float r = std::stof(std::string(vals[0].c_str()));
			float g = std::stof(std::string(vals[1].c_str()));
			float b = std::stof(std::string(vals[2].c_str()));
			float a = 1.0f;
			if (vals.Size() >= 4)
				a = std::stof(std::string(vals[3].c_str()));
			new (&out.uColor) FColor(r, g, b, a);
			out.type = COLOR;
			return out;
		}
	}

	if (inStr.Find("FByteArray{") == 0)
	{
		FString inner = inStr;
		inner.Erase(inner.begin(), inner.begin() + inner.Find("{") + 1);
		inner.EraseAll('}');
		inner.EraseAll(' ');
		auto vals = inner.Split(',');
		new (&out.uByteArray) TArray<uint8>(vals.Size());
		for (int i = 0; i < vals.Size(); i++)
		{
			uint64 value = std::stoull(vals[i].c_str(), nullptr, 16);
			out.uByteArray[i] = (uint8)value;
		}
		return out;
	}

	if (inStr.Find("FStringArray{") == 0)
	{
		FString inner = inStr;
		inner.Erase(inner.begin(), inner.begin() + inner.Find("{") + 1);
		inner.Erase(inner.last());

		new (&out.uStringArray) TArray<FString>();
		bool bInQoutes = false;
		FString v;
		for (int i = 0; i < inner.Size(); i++)
		{
			char ch = inner[i];
			if (!bInQoutes && (ch == ','))
			{
				out.uStringArray.Add(v);
				v.Clear();
			}

			if (bInQoutes && ch == '\\')
			{
				char ch2 = inner[i + 1];
				if (ch2 == '\"' || ch2 == '\'')
				{
					i++;
					v += ch2;
					continue;
				}
			}

			if (ch == '"' || ch == '\'')
			{
				bInQoutes ^= 1;
				continue;
			}

			if (bInQoutes)
				v += ch;
		}
		if (!v.IsEmpty())
			out.uStringArray.Add(v);

		return out;
	}

	if (inStr.IsNumber())
	{
		if (inStr.Find(".") != FString::npos)
		{
			double d = std::stod(std::string(inStr.c_str()));
			out.type = FLOAT;
			out.uFloat = (float)d;
			return out;
		}
		else
		{
			long long i = std::stoll(std::string(inStr.c_str()));
			out.type = INTEGER;
			out.uInteger = (int)i;
			return out;
		}
	}

	FClass* cls = CModuleManager::FindClass(inStr);
	if (cls)
	{
		out.type = CLASS;
		out.uClass = cls;
		return out;
	}

	new (&out.uString) FString(inStr);
	out.type = STRING;
	return out;
}

FVariant& FVariant::operator=(const FVariant& other)
{
	if (other.type != type)
		return *this;

	switch (type)
	{
	case INTEGER:
		uInteger = other.uInteger;
		break;
	case BOOL:
		uBool = other.uBool;
		break;
	case FLOAT:
		uFloat = other.uFloat;
		break;
	case DOUBLE:
		uDouble = other.uDouble;
		break;
	case SIZETYPE:
		uSizeType = other.uSizeType;
		break;
	case OBJ_POINTER:
		uObjPointer = other.uObjPointer;
		break;
	case VECTOR:
		uVector = other.uVector;
		break;
	case COLOR:
		uColor = other.uColor;
		break;
	case STRING:
		uString = other.uString;
		break;
	case CLASS:
		uClass = other.uClass;
		break;
	}

	return *this;
}

bool FVariant::operator==(const FVariant& other) const
{
	if (other.type != type)
		return false;

	switch (type)
	{
	case INTEGER:
		return uInteger == other.uInteger;
	case BOOL:
		return uBool == other.uBool;
	case FLOAT:
		return uFloat == other.uFloat;
	case DOUBLE:
		return uDouble == other.uDouble;
	case SIZETYPE:
		return uSizeType == other.uSizeType;
	case OBJ_POINTER:
		return uObjPointer == other.uObjPointer;
	case VECTOR:
		return uVector == other.uVector;
	case COLOR:
		return uColor == other.uColor;
	case STRING:
		return uString == other.uString;
	case CLASS:
		return uClass == other.uClass;
	}

	return false;
}
