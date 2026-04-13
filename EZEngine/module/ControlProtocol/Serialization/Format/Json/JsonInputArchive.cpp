#include "ControlProtocol/Serialization/Format/Json/JsonInputArchive.h"
#include <cctype>
#include <istream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ControlProtocol::Serialization
{
    namespace
    {
        struct JsonValue
        {
            enum class Type
            {
                Null,
                Bool,
                Number,
                String,
                Object,
                Array
            };

            Type type = Type::Null;
            bool boolValue = false;
            double numberValue = 0.0;
            std::string stringValue;
            std::vector<std::pair<std::string, JsonValue>> objectValues;
            std::vector<JsonValue> arrayValues;

            const JsonValue* Find(const std::string& key) const
            {
                for (const auto& entry : objectValues)
                {
                    if (entry.first == key)
                    {
                        return &entry.second;
                    }
                }
                return nullptr;
            }
        };

        class JsonParser
        {
        public:
            explicit JsonParser(std::string text)
                : m_text(std::move(text))
            {
            }

            std::optional<JsonValue> Parse()
            {
                SkipWhitespace();
                auto result = ParseValue();
                if (!result.has_value())
                {
                    return std::nullopt;
                }

                SkipWhitespace();
                if (m_index != m_text.size())
                {
                    return std::nullopt;
                }
                return result;
            }

        private:
            void SkipWhitespace()
            {
                while (m_index < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_index])) != 0)
                {
                    ++m_index;
                }
            }

            bool Consume(char expected)
            {
                SkipWhitespace();
                if (m_index >= m_text.size() || m_text[m_index] != expected)
                {
                    return false;
                }

                ++m_index;
                return true;
            }

            bool ConsumeKeyword(const char* keyword)
            {
                SkipWhitespace();
                std::size_t start = m_index;
                while (*keyword != '\0')
                {
                    if (m_index >= m_text.size() || m_text[m_index] != *keyword)
                    {
                        m_index = start;
                        return false;
                    }
                    ++m_index;
                    ++keyword;
                }
                return true;
            }

            std::optional<std::string> ParseString()
            {
                SkipWhitespace();
                if (m_index >= m_text.size() || m_text[m_index] != '"')
                {
                    return std::nullopt;
                }

                ++m_index;
                std::string result;
                while (m_index < m_text.size())
                {
                    char ch = m_text[m_index++];
                    if (ch == '"')
                    {
                        return result;
                    }

                    if (ch == '\\')
                    {
                        if (m_index >= m_text.size())
                        {
                            return std::nullopt;
                        }

                        char escape = m_text[m_index++];
                        switch (escape)
                        {
                        case '"': result.push_back('"'); break;
                        case '\\': result.push_back('\\'); break;
                        case 'n': result.push_back('\n'); break;
                        case 'r': result.push_back('\r'); break;
                        case 't': result.push_back('\t'); break;
                        default: return std::nullopt;
                        }
                    }
                    else
                    {
                        result.push_back(ch);
                    }
                }

                return std::nullopt;
            }

            std::optional<double> ParseNumber()
            {
                SkipWhitespace();
                std::size_t start = m_index;
                if (m_index < m_text.size() && (m_text[m_index] == '-' || m_text[m_index] == '+'))
                {
                    ++m_index;
                }

                bool hasDigits = false;
                while (m_index < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_index])) != 0)
                {
                    hasDigits = true;
                    ++m_index;
                }

                if (m_index < m_text.size() && m_text[m_index] == '.')
                {
                    ++m_index;
                    while (m_index < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_index])) != 0)
                    {
                        hasDigits = true;
                        ++m_index;
                    }
                }

                if (m_index < m_text.size() && (m_text[m_index] == 'e' || m_text[m_index] == 'E'))
                {
                    ++m_index;
                    if (m_index < m_text.size() && (m_text[m_index] == '-' || m_text[m_index] == '+'))
                    {
                        ++m_index;
                    }
                    while (m_index < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_index])) != 0)
                    {
                        hasDigits = true;
                        ++m_index;
                    }
                }

                if (!hasDigits)
                {
                    m_index = start;
                    return std::nullopt;
                }

                try
                {
                    return std::stod(m_text.substr(start, m_index - start));
                }
                catch (...)
                {
                    m_index = start;
                    return std::nullopt;
                }
            }

            std::optional<JsonValue> ParseObject()
            {
                if (!Consume('{'))
                {
                    return std::nullopt;
                }

                JsonValue object;
                object.type = JsonValue::Type::Object;

                SkipWhitespace();
                if (Consume('}'))
                {
                    return object;
                }

                while (true)
                {
                    auto key = ParseString();
                    if (!key.has_value() || !Consume(':'))
                    {
                        return std::nullopt;
                    }

                    auto value = ParseValue();
                    if (!value.has_value())
                    {
                        return std::nullopt;
                    }

                    object.objectValues.emplace_back(std::move(*key), std::move(*value));

                    if (Consume('}'))
                    {
                        return object;
                    }

                    if (!Consume(','))
                    {
                        return std::nullopt;
                    }
                }
            }

            std::optional<JsonValue> ParseArray()
            {
                if (!Consume('['))
                {
                    return std::nullopt;
                }

                JsonValue array;
                array.type = JsonValue::Type::Array;

                SkipWhitespace();
                if (Consume(']'))
                {
                    return array;
                }

                while (true)
                {
                    auto value = ParseValue();
                    if (!value.has_value())
                    {
                        return std::nullopt;
                    }

                    array.arrayValues.emplace_back(std::move(*value));

                    if (Consume(']'))
                    {
                        return array;
                    }

                    if (!Consume(','))
                    {
                        return std::nullopt;
                    }
                }
            }

            std::optional<JsonValue> ParseValue()
            {
                SkipWhitespace();
                if (m_index >= m_text.size())
                {
                    return std::nullopt;
                }

                if (m_text[m_index] == '{')
                {
                    return ParseObject();
                }
                if (m_text[m_index] == '[')
                {
                    return ParseArray();
                }
                if (m_text[m_index] == '"')
                {
                    auto text = ParseString();
                    if (!text.has_value())
                    {
                        return std::nullopt;
                    }

                    JsonValue value;
                    value.type = JsonValue::Type::String;
                    value.stringValue = std::move(*text);
                    return value;
                }
                if (ConsumeKeyword("true"))
                {
                    JsonValue value;
                    value.type = JsonValue::Type::Bool;
                    value.boolValue = true;
                    return value;
                }
                if (ConsumeKeyword("false"))
                {
                    JsonValue value;
                    value.type = JsonValue::Type::Bool;
                    value.boolValue = false;
                    return value;
                }
                if (ConsumeKeyword("null"))
                {
                    JsonValue value;
                    value.type = JsonValue::Type::Null;
                    return value;
                }

                auto number = ParseNumber();
                if (!number.has_value())
                {
                    return std::nullopt;
                }

                JsonValue value;
                value.type = JsonValue::Type::Number;
                value.numberValue = *number;
                return value;
            }

            std::string m_text;
            std::size_t m_index = 0;
        };
    }

    class JsonInputArchive::Impl
    {
    public:
        struct ArrayContext
        {
            const JsonValue* array = nullptr;
            std::size_t index = 0;
        };

        explicit Impl(std::istream& stream)
        {
            std::ostringstream buffer;
            buffer << stream.rdbuf();
            JsonParser parser(buffer.str());
            auto parsed = parser.Parse();
            if (parsed.has_value())
            {
                root = std::move(*parsed);
                valid = true;
            }
        }

        const JsonValue* AcquireValue()
        {
            if (pendingValue != nullptr)
            {
                const JsonValue* result = pendingValue;
                pendingValue = nullptr;
                return result;
            }

            if (!arrayStack.empty())
            {
                auto& context = arrayStack.back();
                if (context.array == nullptr || context.index >= context.array->arrayValues.size())
                {
                    return nullptr;
                }

                return &context.array->arrayValues[context.index++];
            }

            if (!rootConsumed)
            {
                rootConsumed = true;
                return &root;
            }

            return nullptr;
        }

        JsonValue root;
        bool valid = false;
        bool rootConsumed = false;
        const JsonValue* pendingValue = nullptr;
        std::vector<const JsonValue*> objectStack;
        std::vector<ArrayContext> arrayStack;
    };

    JsonInputArchive::JsonInputArchive(std::istream& stream)
        : m_impl(std::make_unique<Impl>(stream))
    {
    }

    JsonInputArchive::~JsonInputArchive() = default;

    ArchiveMode JsonInputArchive::GetMode() const noexcept
    {
        return ArchiveMode::Read;
    }

    bool JsonInputArchive::BeginObject(const char* typeName, EZ::u32 version)
    {
        if (!m_impl->valid)
        {
            return false;
        }

        const JsonValue* value = m_impl->AcquireValue();
        if (value == nullptr || value->type != JsonValue::Type::Object)
        {
            return false;
        }

        if (const JsonValue* savedType = value->Find("$type"); savedType != nullptr && savedType->type == JsonValue::Type::String)
        {
            if (savedType->stringValue != (typeName != nullptr ? typeName : ""))
            {
                return false;
            }
        }

        if (const JsonValue* savedVersion = value->Find("$version"); savedVersion != nullptr && savedVersion->type == JsonValue::Type::Number)
        {
            if (static_cast<EZ::u32>(savedVersion->numberValue) != version)
            {
                return false;
            }
        }

        m_impl->objectStack.push_back(value);
        return true;
    }

    void JsonInputArchive::EndObject()
    {
        if (!m_impl->objectStack.empty())
        {
            m_impl->objectStack.pop_back();
        }
    }

    bool JsonInputArchive::BeginField(const char* fieldName)
    {
        if (m_impl->objectStack.empty())
        {
            return false;
        }

        const JsonValue* object = m_impl->objectStack.back();
        const JsonValue* field = object->Find(fieldName != nullptr ? fieldName : "");
        if (field == nullptr)
        {
            return false;
        }

        m_impl->pendingValue = field;
        return true;
    }

    void JsonInputArchive::EndField()
    {
    }

    bool JsonInputArchive::ReadBool(bool& value)
    {
        const JsonValue* node = m_impl->AcquireValue();
        if (node == nullptr || node->type != JsonValue::Type::Bool)
        {
            return false;
        }

        value = node->boolValue;
        return true;
    }

    bool JsonInputArchive::ReadU8(EZ::u8& value)
    {
        EZ::u32 temp = 0;
        if (!ReadU32(temp))
        {
            return false;
        }

        value = static_cast<EZ::u8>(temp);
        return true;
    }

    bool JsonInputArchive::ReadU16(EZ::u16& value)
    {
        EZ::u32 temp = 0;
        if (!ReadU32(temp))
        {
            return false;
        }

        value = static_cast<EZ::u16>(temp);
        return true;
    }

    bool JsonInputArchive::ReadU32(EZ::u32& value)
    {
        const JsonValue* node = m_impl->AcquireValue();
        if (node == nullptr || node->type != JsonValue::Type::Number)
        {
            return false;
        }

        value = static_cast<EZ::u32>(node->numberValue);
        return true;
    }

    bool JsonInputArchive::ReadI32(EZ::i32& value)
    {
        const JsonValue* node = m_impl->AcquireValue();
        if (node == nullptr || node->type != JsonValue::Type::Number)
        {
            return false;
        }

        value = static_cast<EZ::i32>(node->numberValue);
        return true;
    }

    bool JsonInputArchive::ReadF32(EZ::f32& value)
    {
        const JsonValue* node = m_impl->AcquireValue();
        if (node == nullptr || node->type != JsonValue::Type::Number)
        {
            return false;
        }

        value = static_cast<EZ::f32>(node->numberValue);
        return true;
    }

    bool JsonInputArchive::ReadString(std::string& value)
    {
        const JsonValue* node = m_impl->AcquireValue();
        if (node == nullptr || node->type != JsonValue::Type::String)
        {
            return false;
        }

        value = node->stringValue;
        return true;
    }

    bool JsonInputArchive::BeginArray(EZ::u32& count)
    {
        const JsonValue* node = m_impl->AcquireValue();
        if (node == nullptr || node->type != JsonValue::Type::Array)
        {
            return false;
        }

        count = static_cast<EZ::u32>(node->arrayValues.size());
        m_impl->arrayStack.push_back({ node, 0 });
        return true;
    }

    void JsonInputArchive::EndArray()
    {
        if (!m_impl->arrayStack.empty())
        {
            m_impl->arrayStack.pop_back();
        }
    }
}
