#include <unistd.h>
#include <crypt.h>

char *crypt(const char *key, const char *salt)
{
	static char buf[128];
	return crypt_r(key, salt, (struct crypt_data *)buf);
}