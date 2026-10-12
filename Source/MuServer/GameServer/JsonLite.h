#pragma once

// JsonLite: lector JSON mínimo (objetos, arrays, strings, números, true,
// false, null) para los datos de contenido de Data/Custom.  No soporta
// escapes \u: los nombres y rutas del juego son ASCII.

#include <map>
#include <memory>
#include <string>
#include <vector>

class JsonValue
{
public:

	enum Type { Null, Bool, Number, String, Array, Object };

	Type type = Null;

	bool boolean = false;

	double number = 0.0;

	std::string text;

	std::vector<JsonValue> items;

	std::map<std::string, JsonValue> members;

	bool IsObject() const { return this->type == Object; }

	bool IsArray() const { return this->type == Array; }

	bool IsNumber() const { return this->type == Number; }

	bool IsString() const { return this->type == String; }

	// Miembro de un objeto, o nullptr.
	const JsonValue* Get(const char* key) const
	{
		if (this->type != Object)
		{
			return nullptr;
		}

		std::map<std::string, JsonValue>::const_iterator it = this->members.find(key);

		return (it == this->members.end()) ? nullptr : &it->second;
	}

	double GetNumber(const char* key, double fallback) const
	{
		const JsonValue* value = this->Get(key);

		return (value != nullptr && value->IsNumber()) ? value->number : fallback;
	}

	std::string GetString(const char* key, const char* fallback) const
	{
		const JsonValue* value = this->Get(key);

		return (value != nullptr && value->IsString()) ? value->text : std::string(fallback);
	}
};

class JsonParser
{
public:

	// Devuelve false y deja el motivo en `error` si el texto no es JSON válido.
	static bool Parse(const std::string& source, JsonValue& out, std::string& error)
	{
		JsonParser parser(source);

		if (!parser.ParseValue(out))
		{
			error = parser.m_Error;

			return false;
		}

		parser.SkipSpace();

		if (parser.m_Pos != source.size())
		{
			error = parser.Where("texto sobrante después del valor");

			return false;
		}

		return true;
	}

private:

	explicit JsonParser(const std::string& source) : m_Source(source) {}

	std::string Where(const char* message) const
	{
		int line = 1;

		for (size_t n = 0; n < this->m_Pos && n < this->m_Source.size(); n++)
		{
			if (this->m_Source[n] == '\n')
			{
				line++;
			}
		}

		return std::string(message) + " (línea " + std::to_string(line) + ")";
	}

	bool Fail(const char* message)
	{
		if (this->m_Error.empty())
		{
			this->m_Error = this->Where(message);
		}

		return false;
	}

	void SkipSpace()
	{
		while (this->m_Pos < this->m_Source.size())
		{
			char c = this->m_Source[this->m_Pos];

			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			{
				this->m_Pos++;
			}
			else if (c == '/' && this->m_Pos + 1 < this->m_Source.size() && this->m_Source[this->m_Pos + 1] == '/')
			{
				// Comentarios de línea: no son JSON estándar, pero sirven para
				// anotar los archivos de contenido.
				while (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] != '\n')
				{
					this->m_Pos++;
				}
			}
			else
			{
				break;
			}
		}
	}

	bool Match(const char* word)
	{
		size_t length = strlen(word);

		if (this->m_Source.compare(this->m_Pos, length, word) != 0)
		{
			return false;
		}

		this->m_Pos += length;

		return true;
	}

	bool ParseValue(JsonValue& out)
	{
		if (++this->m_Depth > 32)
		{
			return this->Fail("anidamiento demasiado profundo");
		}

		this->SkipSpace();

		if (this->m_Pos >= this->m_Source.size())
		{
			return this->Fail("se esperaba un valor");
		}

		char c = this->m_Source[this->m_Pos];

		bool ok;

		if (c == '{')
		{
			ok = this->ParseObject(out);
		}
		else if (c == '[')
		{
			ok = this->ParseArray(out);
		}
		else if (c == '"')
		{
			out.type = JsonValue::String;

			ok = this->ParseString(out.text);
		}
		else if (this->Match("true"))
		{
			out.type = JsonValue::Bool;

			out.boolean = true;

			ok = true;
		}
		else if (this->Match("false"))
		{
			out.type = JsonValue::Bool;

			ok = true;
		}
		else if (this->Match("null"))
		{
			out.type = JsonValue::Null;

			ok = true;
		}
		else
		{
			ok = this->ParseNumber(out);
		}

		this->m_Depth--;

		return ok;
	}

	bool ParseObject(JsonValue& out)
	{
		out.type = JsonValue::Object;

		this->m_Pos++;

		this->SkipSpace();

		if (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] == '}')
		{
			this->m_Pos++;

			return true;
		}

		while (true)
		{
			this->SkipSpace();

			std::string key;

			if (this->m_Pos >= this->m_Source.size() || this->m_Source[this->m_Pos] != '"' || !this->ParseString(key))
			{
				return this->Fail("se esperaba el nombre de un campo");
			}

			this->SkipSpace();

			if (this->m_Pos >= this->m_Source.size() || this->m_Source[this->m_Pos] != ':')
			{
				return this->Fail("se esperaba ':'");
			}

			this->m_Pos++;

			if (!this->ParseValue(out.members[key]))
			{
				return false;
			}

			this->SkipSpace();

			if (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] == ',')
			{
				this->m_Pos++;

				continue;
			}

			if (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] == '}')
			{
				this->m_Pos++;

				return true;
			}

			return this->Fail("se esperaba ',' o '}'");
		}
	}

	bool ParseArray(JsonValue& out)
	{
		out.type = JsonValue::Array;

		this->m_Pos++;

		this->SkipSpace();

		if (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] == ']')
		{
			this->m_Pos++;

			return true;
		}

		while (true)
		{
			out.items.emplace_back();

			if (!this->ParseValue(out.items.back()))
			{
				return false;
			}

			this->SkipSpace();

			if (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] == ',')
			{
				this->m_Pos++;

				continue;
			}

			if (this->m_Pos < this->m_Source.size() && this->m_Source[this->m_Pos] == ']')
			{
				this->m_Pos++;

				return true;
			}

			return this->Fail("se esperaba ',' o ']'");
		}
	}

	bool ParseString(std::string& out)
	{
		this->m_Pos++;

		while (this->m_Pos < this->m_Source.size())
		{
			char c = this->m_Source[this->m_Pos++];

			if (c == '"')
			{
				return true;
			}

			if (c == '\\')
			{
				if (this->m_Pos >= this->m_Source.size())
				{
					break;
				}

				char e = this->m_Source[this->m_Pos++];

				switch (e)
				{
					case 'n': out += '\n'; break;
					case 't': out += '\t'; break;
					case 'r': out += '\r'; break;
					case '"': out += '"'; break;
					case '\\': out += '\\'; break;
					case '/': out += '/'; break;
					default: return this->Fail("escape no soportado");
				}

				continue;
			}

			out += c;
		}

		return this->Fail("string sin cerrar");
	}

	bool ParseNumber(JsonValue& out)
	{
		const char* begin = this->m_Source.c_str() + this->m_Pos;

		char* end = nullptr;

		double value = strtod(begin, &end);

		if (end == begin)
		{
			return this->Fail("valor inválido");
		}

		out.type = JsonValue::Number;

		out.number = value;

		this->m_Pos += (size_t)(end - begin);

		return true;
	}

	const std::string& m_Source;

	size_t m_Pos = 0;

	int m_Depth = 0;

	std::string m_Error;
};
