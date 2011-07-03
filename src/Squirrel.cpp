///////////////////////////////////////////////////////////////////////
// Wendy Squirrel bindings - based on Sqrat
// Copyright (c) 2009 Brandon Jones
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>
#include <wendy/Core.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

#include <wendy/Squirrel.h>

#include <sqstdaux.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace sq
  {

///////////////////////////////////////////////////////////////////////

VM::VM(ResourceIndex& initIndex):
  index(initIndex),
  vm(NULL)
{
  vm = sq_open(1024);

  sq_setforeignptr(vm, this);
  sq_setprintfunc(vm, onLogMessage, onLogError);
  sq_setcompilererrorhandler(vm, onCompilerError);

  sq_pushroottable(vm);
  sqstd_register_mathlib(vm);
  sqstd_register_stringlib(vm);
  sq_newclosure(vm, onRuntimeError, 0);
  sq_seterrorhandler(vm);
  sq_poptop(vm);
}

VM::~VM(void)
{
  if (vm)
    sq_close(vm);
}

bool VM::execute(const Path& path)
{
  std::ifstream stream;
  if (!index.openFile(stream, path))
    return false;

  stream.seekg(0, std::ios::end);

  String text;
  text.resize((unsigned int) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  return execute(path.asString().c_str(), text.c_str());
}

bool VM::execute(const char* name, const char* text)
{
  if (SQ_FAILED(sq_compilebuffer(vm, text, std::strlen(text), name, true)))
    return false;

  sq_pushroottable(vm);
  if (SQ_FAILED(sq_call(vm, 1, false, true)))
    return false;
  sq_poptop(vm);

  return true;
}

VM::operator HSQUIRRELVM (void)
{
  return vm;
}

Table VM::getRootTable(void)
{
  sq_pushroottable(vm);
  Table table(vm, -1);
  sq_poptop(vm);

  return table;
}

Table VM::getRegistryTable(void)
{
  sq_pushregistrytable(vm);
  Table table(vm, -1);
  sq_poptop(vm);

  return table;
}

ResourceIndex& VM::getIndex(void) const
{
  return index;
}

void VM::onLogMessage(HSQUIRRELVM vm, const SQChar* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  log("%s", message);

  std::free(message);
}

void VM::onLogError(HSQUIRRELVM vm, const SQChar* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  logError("%s", message);

  std::free(message);
}

void VM::onCompilerError(HSQUIRRELVM vm,
                         const SQChar* description,
                         const SQChar* source,
                         SQInteger line,
                         SQInteger column)
{
  logError("%s:%i:%i: %s", source, line, column, description);
}

SQInteger VM::onRuntimeError(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) >= 1)
    sqstd_printcallstack(vm);

  return 0;
}

///////////////////////////////////////////////////////////////////////

Object::Object(void):
  vm(NULL)
{
  sq_resetobject(&handle);
}

Object::Object(HSQUIRRELVM initVM, SQInteger index):
  vm(initVM)
{
  sq_resetobject(&handle);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
}

Object::Object(const Object& source):
  vm(source.vm),
  handle(source.handle)
{
  sq_addref(vm, &handle);
}

Object::~Object(void)
{
  sq_release(vm, &handle);
}

Object& Object::operator = (const Object& source)
{
  sq_release(vm, &handle);
  handle = source.handle;
  vm = source.vm;
  sq_addref(vm, &handle);
  return *this;
}

bool Object::isNull(void) const
{
  return sq_isnull(handle);
}

bool Object::isArray(void) const
{
  return sq_isarray(handle);
}

bool Object::isTable(void) const
{
  return sq_istable(handle);
}

bool Object::isClass(void) const
{
  return sq_isclass(handle);
}

String Object::asString(void) const
{
  sq_pushobject(vm, handle);
  sq_tostring(vm, -1);
  String result = Value<String>::get(vm, -1);
  sq_pop(vm, 2);
  return result;
}

HSQOBJECT Object::getHandle(void)
{
  return handle;
}

HSQUIRRELVM Object::getVM(void) const
{
  return vm;
}

Object::Object(HSQUIRRELVM initVM):
  vm(initVM)
{
  sq_resetobject(&handle);
}

void Object::addFunction(const char* name,
                         void* pointer,
                         size_t pointerSize,
                         SQFUNCTION function,
                         bool staticMember)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  std::memcpy(sq_newuserdata(vm, pointerSize), pointer, pointerSize);
  sq_newclosure(vm, function, 1);

  sq_newslot(vm, -3, staticMember);
  sq_poptop(vm);
}

///////////////////////////////////////////////////////////////////////

Array::Array(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newarray(vm, 0);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
  sq_poptop(vm);
}

Array::Array(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isArray())
    throw Exception("Object is not an array");
}

void Array::remove(SQInteger index)
{
  sq_pushobject(vm, handle);
  sq_arrayremove(vm, -1, index);
  sq_poptop(vm);
}

void Array::pop(void)
{
  sq_pushobject(vm, handle);
  sq_arraypop(vm, -1, false);
  sq_poptop(vm);
}

void Array::resize(SQInteger newSize)
{
  sq_pushobject(vm, handle);
  sq_arrayresize(vm, -1, newSize);
  sq_poptop(vm);
}

void Array::reverse(void)
{
  sq_pushobject(vm, handle);
  sq_arrayreverse(vm, -1);
  sq_poptop(vm);
}

void Array::clear(void)
{
  sq_pushobject(vm, handle);
  sq_clear(vm, -1);
  sq_poptop(vm);
}

Object Array::operator [] (SQInteger index) const
{
  sq_pushobject(vm, handle);
  sq_pushinteger(vm, index);
  if (SQ_FAILED(sq_get(vm, -2)))
    throw Exception("No array element at index");
  Object result(vm, -1);
  sq_pop(vm, 2);
  return result;
}

SQInteger Array::getSize(void) const
{
  sq_pushobject(vm, handle);
  SQInteger size = sq_getsize(vm, -1);
  sq_poptop(vm);
  return size;
}

///////////////////////////////////////////////////////////////////////

Table::Table(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newtable(vm);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
  sq_poptop(vm);
}

Table::Table(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isTable())
    throw Exception("Object is not a table");
}

bool Table::removeSlot(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  const SQRESULT result = sq_deleteslot(vm, -2, false);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

void Table::clear(void)
{
  sq_pushobject(vm, handle);
  sq_clear(vm, -1);
  sq_poptop(vm);
}

SQInteger Table::getSize(void) const
{
  sq_pushobject(vm, handle);
  SQInteger size = sq_getsize(vm, -1);
  sq_poptop(vm);
  return size;
}

///////////////////////////////////////////////////////////////////////

Class::Class(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newtable(vm);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
  sq_poptop(vm);
}

Class::Class(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isClass())
    throw Exception("Object is not a class");
}

bool Class::removeSlot(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  const SQRESULT result = sq_deleteslot(vm, -2, false);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

void Class::clear(void)
{
  sq_pushobject(vm, handle);
  sq_clear(vm, -1);
  sq_poptop(vm);
}

SQInteger Class::getSize(void) const
{
  sq_pushobject(vm, handle);
  SQInteger size = sq_getsize(vm, -1);
  sq_poptop(vm);
  return size;
}

Class::Class(void)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace sq*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////