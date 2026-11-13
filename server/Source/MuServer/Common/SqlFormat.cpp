#include "SqlFormat.h"

bool SqlFormat(std::string& out, const char* format, va_list args, const SqlEscapeFn& escape, std::string& error)
{
	out.clear();

	if (format == nullptr)
	{
		error = "formato nulo";
		return false;
	}

	for (const char* p = format; *p != 0; p++)
	{
		if (*p != '%')
		{
			out += *p;
			continue;
		}

		switch (*(++p))
		{
			case '%':
				out += '%';
				break;

			case 'd':
				out += std::to_string(va_arg(args, int));
				break;

			case 'u':
				out += std::to_string(va_arg(args, unsigned int));
				break;

			case 's':
			{
				// El escape sólo protege dentro de un literal: un %s sin
				// comillas dejaría que el valor agregue SQL propio aunque
				// esté escapado.  Se exige '%s' en el formato.
				if (p - format < 2 || p[-2] != '\'' || p[1] != '\'')
				{
					error = "%s fuera de comillas simples (tiene que ser '%s')";
					return false;
				}

				const char* value = va_arg(args, const char*);

				if (value == nullptr)
				{
					error = "valor nulo para %s";
					return false;
				}

				std::string escaped;

				if (!escape(value, escaped))
				{
					error = "no se pudo escapar un valor";
					return false;
				}

				out += escaped;
				break;
			}

			default:
				error = std::string("conversion no soportada '%") + ((*p != 0) ? std::string(1, *p) : std::string()) + "' (solo %d, %u, %s y %%)";
				return false;
		}
	}

	return true;
}
