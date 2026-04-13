#pragma once
#ifndef __C_P_SERIALIZATION_SERIALIZEOBJECT_H__
#define __C_P_SERIALIZATION_SERIALIZEOBJECT_H__

#include "ControlProtocol/Serialization/Archive/IInputArchive.h"
#include "ControlProtocol/Serialization/Archive/IOutputArchive.h"
#include "ControlProtocol/Serialization/Traits/Reflectable.h"
#include "ControlProtocol/Serialization/Traits/ProtocolTraits.h"

namespace ControlProtocol::Serialization
{
	template<typename T>
	bool SerializeValue(IOutputArchive& archive, T& value);

	template<typename T>
	bool DeserializeValue(IInputArchive& archive, T& value);

	template<typename T>
	bool SerializeObject(IOutputArchive& archive, T& object)
	{
		static_assert(IsReflectableV<T>, "SerializeObject requires a reflectable protocol type.");

		if (!archive.BeginObject(ProtocolTraits<T>::TypeName(), ProtocolTraits<T>::Version()))
		{
			return false;
		}

		bool ok = true;

		object.Reflect([&](const char* fieldName, auto& fieldValue)
			{
				if (!ok) return;

				if (!archive.BeginField(fieldName))
				{
					ok = false;
					return;
				}

				ok = SerializeValue(archive, fieldValue);
				archive.EndField();
			});

		archive.EndObject();
		return ok;
	}

	template<typename T>
	bool DeserializeObject(IInputArchive& archive, T& object)
	{
		static_assert(IsReflectableV<T>, "DeserializeObject requires a reflectable protocol type.");

		if (!archive.BeginObject(ProtocolTraits<T>::TypeName(), ProtocolTraits<T>::Version()))
		{
			return false;
		}

		bool ok = true;

		object.Reflect([&](const char* fieldName, auto& fieldValue)
			{
				if (!ok) return;

				if (!archive.BeginField(fieldName))
				{
					ok = false;
					return;
				}

				ok = DeserializeValue(archive, fieldValue);
				archive.EndField();
			});

		archive.EndObject();
		return ok;
	}
}

#endif