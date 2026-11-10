#pragma once

// Contraseñas de cuenta con bcrypt ("$2b$", 60 caracteres ASCII). El mismo
// formato que genera bcryptjs en el panel web, así que cualquiera de los dos
// puede crear o verificar el hash del otro.
namespace PasswordHash
{
	constexpr int HashLength = 60;
	constexpr int DefaultCost = 10;
	constexpr int MinCost = 4;
	constexpr int MaxCost = 15;

	// Lo guardado en memb__pwd es un hash bcrypt (y no la contraseña en claro
	// ni un MD5). `size` es el tamaño del buffer; tiene que tener un cero en
	// la posición 60 o después de los 60 caracteres.
	bool IsHash(const unsigned char* stored, int size);

	// Genera el hash con sal aleatoria. `out` necesita HashLength + 1 bytes.
	bool Make(const char* password, int cost, char* out, int outSize);

	// Compara en tiempo constante.
	bool Check(const char* password, const char* hash);
}
