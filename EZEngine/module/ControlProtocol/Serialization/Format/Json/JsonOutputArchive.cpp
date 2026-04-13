#include "ControlProtocol/Serialization/Format/Json/JsonOutputArchive.h"
#include <ostream>
#include <string_view>
#include <vector>

namespace ControlProtocol::Serialization
{
    class JsonOutputArchive::Impl
    {
    public:
        enum class ContextType
        {
            Object,
            Array
        };

        struct Context
        {
            ContextType type = ContextType::Object;
            bool first = true;
            bool fieldAwaitingValue = false;
        };

        explicit Impl(std::ostream& output)
            : stream(output)
        {
        }

        bool PrepareValue()
        {
            if (contexts.empty())
            {
                return stream.good();
            }

            Context& context = contexts.back();
            if (context.type == ContextType::Array)
            {
                if (!context.first)
                {
                    stream << ',';
                }
                context.first = false;
                return stream.good();
            }

            if (!context.fieldAwaitingValue)
            {
                return false;
            }

            context.fieldAwaitingValue = false;
            return stream.good();
        }

        void WriteEscapedString(std::string_view text)
        {
            stream << '"';
            for (char ch : text)
            {
                switch (ch)
                {
                case '"': stream << "\\\""; break;
                case '\\': stream << "\\\\"; break;
                case '\n': stream << "\\n"; break;
                case '\r': stream << "\\r"; break;
                case '\t': stream << "\\t"; break;
                default: stream << ch; break;
                }
            }
            stream << '"';
        }

        std::ostream& stream;
        std::vector<Context> contexts;
    };

    JsonOutputArchive::JsonOutputArchive(std::ostream& stream)
        : m_impl(std::make_unique<Impl>(stream))
    {
    }

    JsonOutputArchive::~JsonOutputArchive() = default;

    ArchiveMode JsonOutputArchive::GetMode() const noexcept
    {
        return ArchiveMode::Write;
    }

    bool JsonOutputArchive::BeginObject(const char* typeName, EZ::u32 version)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->stream << '{';
        m_impl->contexts.push_back({ Impl::ContextType::Object, true, false });

        BeginField("$type");
        std::string type = typeName != nullptr ? typeName : "";
        WriteString(type);
        EndField();

        BeginField("$version");
        WriteU32(version);
        EndField();
        return m_impl->stream.good();
    }

    void JsonOutputArchive::EndObject()
    {
        m_impl->stream << '}';
        if (!m_impl->contexts.empty())
        {
            m_impl->contexts.pop_back();
        }
    }

    bool JsonOutputArchive::BeginField(const char* fieldName)
    {
        if (m_impl->contexts.empty())
        {
            return false;
        }

        auto& context = m_impl->contexts.back();
        if (context.type != Impl::ContextType::Object)
        {
            return false;
        }

        if (!context.first)
        {
            m_impl->stream << ',';
        }
        context.first = false;
        m_impl->WriteEscapedString(fieldName != nullptr ? fieldName : "");
        m_impl->stream << ':';
        context.fieldAwaitingValue = true;
        return m_impl->stream.good();
    }

    void JsonOutputArchive::EndField()
    {
    }

    bool JsonOutputArchive::WriteBool(bool value)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->stream << (value ? "true" : "false");
        return m_impl->stream.good();
    }

    bool JsonOutputArchive::WriteU8(EZ::u8 value)
    {
        return WriteU32(static_cast<EZ::u32>(value));
    }

    bool JsonOutputArchive::WriteU16(EZ::u16 value)
    {
        return WriteU32(static_cast<EZ::u32>(value));
    }

    bool JsonOutputArchive::WriteU32(EZ::u32 value)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->stream << value;
        return m_impl->stream.good();
    }

    bool JsonOutputArchive::WriteI32(EZ::i32 value)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->stream << value;
        return m_impl->stream.good();
    }

    bool JsonOutputArchive::WriteF32(EZ::f32 value)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->stream << value;
        return m_impl->stream.good();
    }

    bool JsonOutputArchive::WriteString(const std::string& value)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->WriteEscapedString(value);
        return m_impl->stream.good();
    }

    bool JsonOutputArchive::BeginArray(EZ::u32 /*count*/)
    {
        if (!m_impl->PrepareValue())
        {
            return false;
        }

        m_impl->stream << '[';
        m_impl->contexts.push_back({ Impl::ContextType::Array, true, false });
        return m_impl->stream.good();
    }

    void JsonOutputArchive::EndArray()
    {
        m_impl->stream << ']';
        if (!m_impl->contexts.empty())
        {
            m_impl->contexts.pop_back();
        }
    }
}
