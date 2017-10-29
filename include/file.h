#pragma once
#include <string>

namespace suex::file {
double Size(const std::string &path);

void Remove(const std::string &path, bool silent = false);

bool IsSecure(const std::string &path);

void Secure(const std::string &path);

void Clone(const std::string &from, const std::string &to, bool secure = false);

void Create(const std::string &path, bool secure = false);

int PermissionBits(const std::string& path);
}

