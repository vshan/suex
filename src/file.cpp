#include <file.h>
#include <sys/stat.h>
#include <exceptions.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <path.h>
#include <fstream>

using namespace suex;
using namespace suex::utils;

void file::Secure(const std::string &path) {
  // chmod 440
  if (chmod(path.c_str(), S_IRUSR | S_IRGRP) < 0) {
    throw suex::PermissionError(std::strerror(errno));
  }

  // chown root:root
  if (chown(path.c_str(), 0, 0) < 0) {
    throw suex::PermissionError(std::strerror(errno));
  }
}

double file::Size(const std::string &path) {
  struct stat st{};
  if (stat(path.c_str(), &st) != 0) {
    throw IOError("could not find the file specified");
  }
  return st.st_size / 1024.0;
}

void file::Remove(const std::string &path, bool silent) {
  if (!path::Exists(path)) {
    return;
  }

  if (remove(path.c_str()) != 0 && !silent) {
    throw suex::IOError("%s: %s", path.c_str(), std::strerror(errno));
  }
}

void file::Clone(const std::string &from, const std::string &to, bool secure) {
  int src_fd = open(from.c_str(), O_RDONLY, 0);
  if (src_fd == -1) {
    throw suex::IOError("%s: %s", from.c_str(), std::strerror(errno));
  }
  DEFER(close(src_fd));

  int dst_fd = open(to.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0440);
  if (dst_fd == -1) {
    throw suex::IOError("%s: %s", to.c_str(), std::strerror(errno));
  }
  DEFER(close(dst_fd));

  // only secure the file if it should be secured
  if (secure && fchown(dst_fd, 0, 0) < 0) {
    throw suex::IOError("%s: %s", to.c_str(), std::strerror(errno));
  }

  struct stat st{};
  if (fstat(src_fd, &st) < 0) {
    throw suex::IOError("%s: %s", to.c_str(), std::strerror(errno));
  }

  if (sendfile(dst_fd, src_fd, nullptr, (size_t) st.st_size) <= 0) {
    throw suex::IOError("%s: %s", to.c_str(), std::strerror(errno));
  }
}

void file::Create(const std::string &path, bool secure) {
  if (utils::path::Exists(path)) {
    throw IOError("file '%s' already exists", path.c_str());
  }

  if (secure) {
    Secure(path);
  }

}
bool ::suex::file::IsSecure(const std::string &path) {
  struct stat st{};
  if (stat(path.c_str(), &st) != 0) {
    throw IOError("could not find the file specified");
  }

  if (file::PermissionBits(path) != 440) {
    return false;
  }

  // owner should be root:root
  return st.st_uid == 0 && st.st_gid == 0;
}

int ::suex::file::PermissionBits(const std::string &path) {
  struct stat st{};
  if (stat(path.c_str(), &st) != 0) {
    throw IOError("could not find the file specified");
  }
  // return permission bits in a "human readable" format
  int user = (st.st_mode & S_IRWXU) >> 6;
  int group = (st.st_mode & S_IRWXG) >> 3;
  int others = st.st_mode & S_IRWXO;
  return (user * 100) + (group * 10) + others;
}
