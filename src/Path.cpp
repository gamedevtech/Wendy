///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Path.hpp>

#if WENDY_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if WENDY_HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if WENDY_HAVE_DIRECT_H
#include <direct.h>
#endif

#if WENDY_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if WENDY_HAVE_DIRENT_H
#include <dirent.h>
#endif

#if WENDY_HAVE_WINDOWS_H
#include <windows.h>
#endif

#if WENDY_HAVE_IO_H
#include <io.h>
#endif

#include <cstdio>
#include <cstdlib>

#ifdef _MSC_VER

#define F_OK 0
#define W_OK 2
#define R_OK 4

#define S_ISDIR(x) ((x) & _S_IFDIR)
#define S_ISREG(x) ((x) & _S_IFREG)

#endif /*_MSC_VER*/

namespace wendy
{

Path::Path(const std::string& name)
{
  operator = (name);
}

bool Path::createDirectory() const
{
#if WENDY_SYSTEM_WIN32
  return _mkdir(m_string.c_str()) == 0;
#else
  return mkdir(m_string.c_str(), 0777) == 0;
#endif
}

bool Path::destroyDirectory() const
{
#if WENDY_SYSTEM_WIN32
  return _rmdir(m_string.c_str()) == 0;
#else
  return rmdir(m_string.c_str()) == 0;
#endif
}

bool Path::rename(const std::string& target)
{
  return ::rename(m_string.c_str(), target.c_str()) == 0;
}

bool Path::remove()
{
  return unlink(m_string.c_str()) == 0;
}

bool Path::exists() const
{
#if WENDY_SYSTEM_WIN32
  return _access(m_string.c_str(), F_OK) == 0;
#else
  return access(m_string.c_str(), F_OK) == 0;
#endif
}

Path Path::operator + (const std::string& child) const
{
  return Path(m_string + '/' + child);
}

Path& Path::operator += (const std::string& child)
{
  return operator = (m_string + '/' + child);
}

bool Path::operator == (const Path& other) const
{
  // TODO: Implement sane code.

  return m_string == other.m_string;
}

bool Path::operator != (const Path& other) const
{
  // TODO: Implement sane code.

  return m_string != other.m_string;
}

Path& Path::operator = (const std::string& newPath)
{
  m_string = newPath;

  if (!m_string.empty())
  {
    // Remove extraneous trailing slashes

    while (std::string::size_type end = m_string.length() - 1)
    {
      if (m_string[end] != '/')
        break;

      m_string.erase(end);
    }

    // TODO: Compact repeated slashes.
  }

  return *this;
}

bool Path::isReadable() const
{
#if WENDY_SYSTEM_WIN32
  return _access(m_string.c_str(), R_OK) == 0;
#else
  return access(m_string.c_str(), R_OK) == 0;
#endif
}

bool Path::isWritable() const
{
#if WENDY_SYSTEM_WIN32
  return _access(m_string.c_str(), W_OK) == 0;
#else
  return access(m_string.c_str(), W_OK) == 0;
#endif
}

bool Path::isFile() const
{
#if WENDY_SYSTEM_WIN32
  struct _stati64 sb;

  if (_stati64(m_string.c_str(), &sb) != 0)
    return false;
#else
  struct stat64 sb;

  if (stat64(m_string.c_str(), &sb) != 0)
    return false;
#endif

  return S_ISREG(sb.st_mode) ? true : false;
}

bool Path::isDirectory() const
{
#if WENDY_SYSTEM_WIN32
  struct _stati64 sb;

  if (_stati64(m_string.c_str(), &sb) != 0)
    return false;
#else
  struct stat64 sb;

  if (stat64(m_string.c_str(), &sb) != 0)
    return false;
#endif

  return S_ISDIR(sb.st_mode) ? true : false;
}

Path Path::parent() const
{
  // TODO: Fix this.

  std::string::size_type offset = m_string.find_last_of('/');
  if (offset == std::string::npos)
    return Path(".");

  return Path(m_string.substr(0, offset + 1));
}

std::vector<std::string> Path::children() const
{
  std::vector<std::string> children;

#if WENDY_SYSTEM_WIN32
  WIN32_FIND_DATA data;
  HANDLE search;

  search = FindFirstFile((m_string + "/*").c_str(), &data);
  if (search != INVALID_HANDLE_VALUE)
  {
    do
    {
      children.push_back(data.cFileName);
    }
    while (FindNextFile(search, &data));

    FindClose(search);
  }
#else
  DIR* stream;
  dirent* entry;

  stream = opendir(m_string.c_str());
  if (stream)
  {
    while ((entry = readdir(stream)))
      children.push_back(entry->d_name);

    closedir(stream);
  }
#endif

  return children;
}

std::vector<std::string> Path::childrenMatching(const std::regex& regex) const
{
  std::vector<std::string> children;
  std::cmatch match;

#if WENDY_SYSTEM_WIN32
  WIN32_FIND_DATA data;
  HANDLE search;

  search = FindFirstFile((m_string + "/*").c_str(), &data);
  if (search != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (std::regex_match(data.cFileName, match, regex))
        children.push_back(data.cFileName);
    }
    while (FindNextFile(search, &data));

    FindClose(search);
  }
#else
  DIR* stream;
  dirent* entry;

  stream = opendir(m_string.c_str());
  if (stream)
  {
    while ((entry = readdir(stream)))
    {
      if (std::regex_match(entry->d_name, match, regex))
        children.push_back(entry->d_name);
    }

    closedir(stream);
  }
#endif

  return children;
}

std::string Path::suffix() const
{
  std::string last;

  std::string::size_type start = m_string.find_last_of('/');
  if (start == std::string::npos)
    last = m_string;
  else
    last = m_string.substr(start, std::string::npos);

  std::string::size_type offset = last.find_last_of('.');
  if (offset == std::string::npos)
    return "";

  return last.substr(offset + 1, std::string::npos);
}

std::string Path::basename() const
{
  std::string::size_type start = m_string.find_last_of('/');
  std::string::size_type end = m_string.find_last_of('.');

  if (start == std::string::npos)
    start = 0;
  else
    start++;

  if (end != std::string::npos && end < start)
    end = std::string::npos;

  return m_string.substr(start, end - start);
}

} /*namespace wendy*/

